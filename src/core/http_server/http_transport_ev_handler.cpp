#include "http_transport_ev_handler.hpp"
#include "replies_types.hpp"
#include "http_server_core_config.hpp"
#include "hcore_notification_recv.hpp" 
#include "core_notification_center.hpp"

#include <boost/assert.hpp>

#define T2H_CONTENT_REQUEST_SYNCED

/** 
 * Public transport_ev_handler api 
 */

namespace t2h_core {

namespace details {

static inline void t2h_http_fingerprint(utility::fingerprint & fp) 
{
	// TODO add fingerprint funtionality
}

} // details

transport_ev_handler::transport_ev_handler(setting_manager_ptr setting_manager) 
	: base_transport_ev_handler(), 
	setting_manager_(setting_manager), 
	settings_(),	
	request_parser_(),
	hcore_recv_()
{	
	BOOST_ASSERT(setting_manager_ != NULL);
	
	settings_.doc_root = setting_manager_->get_value<std::string>("doc_root");
	BOOST_ASSERT(settings_.doc_root.size() != 0);

	settings_.max_allowed_size_for_reply = 1 * (boost::int64_t)setting_manager_->get_value<std::size_t>("hc_max_allowed_size_for_reply");
	BOOST_ASSERT(settings_.max_allowed_size_for_reply != 0);
	
	settings_.max_sync_timeout = setting_manager_->get_value<std::size_t>("hc_max_sync_timeout");	
	details::t2h_http_fingerprint(settings_.fp);

	hcore_recv_ = common::notification_receiver_ptr(new hcore_notification_recv(*this));
	core_notification_center()->add_notification_receiver(hcore_recv_);
}

transport_ev_handler::~transport_ev_handler() 
{
	core_notification_center()->remove_notification_receiver("hcore_notification_recv");
}

transport_ev_handler::recv_result transport_ev_handler::on_recv(
	base_transport_ev_handler::recv_buffer_type const & recv_data, 
	std::size_t recv_data_size, 
	buffer_type & answ_data) 
{
	using namespace utility;
	
	http_request client_request;

	boost::tribool result = parse_recv(client_request, recv_data, recv_data_size);
	if (result)  
		return proceed_execute_data(client_request, answ_data);
	else if (!result) {
		HCORE_WARNING("parsing of the recv data failed")
		return error(answ_data, http_reply::bad_request);
	}
	return more_data(answ_data);
}

void transport_ev_handler::on_error(int error) 
{
	HCORE_WARNING("error code '%i'", error)
	// TODO impl this funtionality
}

void transport_ev_handler::on_close() 
{
	HCORE_TRACE("connection closed")
	// TODO impl this funtionality
}

transport_ev_handler::ptr_type transport_ev_handler::clone() 
{
	// TODO Think about move semantics ...
	using namespace common;
	return base_transport_ev_handler_ptr(new transport_ev_handler(setting_manager_));
}

void transport_ev_handler::on_file_add(
		std::string const & file_path, 
		boost::int64_t file_size, 
		boost::int64_t avaliable_bytes) 
{
	on_file_update(file_path, file_size, avaliable_bytes);
}

void transport_ev_handler::on_file_remove(std::string const & file_path) 
{
	file_info_buffer_.remove_info(file_path);
}

void transport_ev_handler::on_file_update(
		std::string const & file_path, 
		boost::int64_t file_size, 
		boost::int64_t avaliable_bytes) 
{
	details::hc_file_info_ptr finfo(new details::hc_file_info()); 
	finfo->file_path = file_path; 
	finfo->file_size = file_size;
	finfo->avaliable_bytes = avaliable_bytes;
	file_info_buffer_.update_info(finfo);
}

/** 
 * Private transport_ev_handler api 
 */
boost::tribool transport_ev_handler::parse_recv(
	utility::http_request & request, base_transport_ev_handler::recv_buffer_type const & data, std::size_t data_size)
{
	boost::tribool result;
	request_parser_.reset_state();
	boost::tie(result, boost::tuples::ignore) 
		= request_parser_.parse(request, data.data(), data.data() + data_size);
	return result;
}

transport_ev_handler::recv_result transport_ev_handler::proceed_execute_data(
	utility::http_request const & req, buffer_type & reply_buf) 
{
	using namespace utility;
	
	std::string request_path;
	
	if (!url_decode(req.uri, request_path)) {
		HCORE_WARNING("uri decode failed with data '%s'", req.uri.c_str())
		return error(reply_buf, http_reply::bad_request);
	}

	if (request_path.find("..") != std::string::npos)
		return error(reply_buf, http_reply::bad_request);

	if (is_root(request_path))  
		return root_request(reply_buf);
	
	request_path = settings_.doc_root + request_path;
	return dispatch_client_request(request_path, req, reply_buf);
}

transport_ev_handler::recv_result transport_ev_handler::more_data(buffer_type & reply_buf) 
{
	using namespace utility; 
	// TODO impl this funtionality
	return base_transport_ev_handler::more_data;         
}

transport_ev_handler::recv_result transport_ev_handler::error(
	buffer_type & reply_buf, utility::http_reply::status_type status) 
{
	using namespace utility;
	http_reply serv_reply(reply_buf);
	serv_reply.stock_reply(status);
	return base_transport_ev_handler::bad_data;
}

/**
 * dispatching of the client request
 */
transport_ev_handler::recv_result transport_ev_handler::dispatch_client_request(
	std::string const & req_path, 
	utility::http_request const & req, 
	buffer_type & reply_buf) 
{
	using namespace utility;

	switch (req.mtype) 
	{
		case http_request::mget :
			return on_mget_request(req_path, req, reply_buf);
		case http_request::mhead :
			return on_mhead_request(req_path, reply_buf);
		case http_request::mpost : 
			return on_mpost_request(req_path, req, reply_buf);
		case http_request::munknown : break; 
		default : break;
	} // switch

	HCORE_WARNING("req.mtype == http_request::munknown for '%s'", req_path.c_str())
	return error(reply_buf, http_reply::bad_request);	
}

transport_ev_handler::recv_result transport_ev_handler::on_mget_request(
	std::string const & request_path, utility::http_request const & req, buffer_type & reply_buf)
{
	using namespace utility;

	range_header rheader;
	bool sync_state = false;
	details::hc_file_info_ptr file_info;
	
	if (http_translate_range_header(rheader, req.headers)) {
#if defined(T2H_DEBUG)
	HCORE_TRACE("Partial content request, for path '%s'", request_path.c_str())
#endif // T2H_DEBUG
		/*	Sync with file system, follow logic has two bad cases : 
		 	first is file not in buffer(means not exist ot was not added via notification center), 
			second is sync with filesystem failed(network problem etc...) */	
		boost::tie(sync_state, file_info) = sync_request_with_buffer(rheader, request_path); 
		if (!sync_state && !file_info) {
			HCORE_WARNING("'%s' path not found", request_path.c_str())
			return error(reply_buf, http_reply::not_found);
		} else if (!sync_state && file_info) {
			HCORE_WARNING("for path '%s' sync with filesystem failed", request_path.c_str())
			return error(reply_buf, http_reply::internal_server_error);
		} // else if
		
		details::partial_content_reply_param const pcr_param 
			= details::create_partial_content_param(request_path, file_info->file_size, rheader.bstart_1, rheader.bend_1);
		details::partial_content_reply pcr_reply(reply_buf, pcr_param);
		if (!pcr_reply.do_formatting_reply()) {
			HCORE_WARNING("preparing reply failed for req. path '%s'", request_path.c_str())
			return error(reply_buf, http_reply::internal_server_error);
		} // if
	} else {

#if defined(T2H_DEBUG)
		HCORE_TRACE("Content request, for path '%s'", request_path.c_str())
#endif // T2H_DEBUG

#if defined(T2H_CONTENT_REQUEST_SYNCED)
		/*  If client request a file content but the content a large for sending in single reply, 
		 	so just do a reply via partial request(From begining to allowed_size_for_reading). */
		rheader.bstart_1 = rheader.bend_1 = range_header::all;
		boost::tie(sync_state, file_info) = sync_request_with_buffer(rheader, request_path);
		if (!sync_state && !file_info) {
			HCORE_WARNING("'%s' path not found", request_path.c_str())
			return error(reply_buf, http_reply::not_found);
		} else if (!sync_state && file_info) {
			HCORE_WARNING("for path '%s' sync with filesystem failed", request_path.c_str())
			return error(reply_buf, http_reply::internal_server_error);
		} // else if
		
		details::partial_content_reply_param const pcr_param 
			= details::create_partial_content_param(request_path, file_info->file_size, rheader.bstart_1, rheader.bend_1);
		details::partial_content_reply pcr_reply(reply_buf, pcr_param);
		if (!pcr_reply.do_formatting_reply()) {
			HCORE_WARNING("preparing reply failed for req. path '%s'", request_path.c_str())
			return error(reply_buf, http_reply::internal_server_error);
		} // if
#else 
		/*  If range header not have range header just send file; 
			NOTE : Next behavior is off, because VLC player(last tested 2.0.2) has bug/weirdness at the GET request send; */
		boost::system::error_code fs_error;
		details::send_content_reply_param const scrp = { request_path, boost::filesystem::file_size(request_path, fs_error) };
		if (fs_error) {
			HCORE_WARNING("path '%s' not exist", request_path.c_str())
			return error(reply_buf, http_reply::not_found);
		}
		details::send_content_reply sc_reply(reply_buf, scrp);
		if (!sc_reply.do_formatting_reply()) {
			HCORE_WARNING("preparing reply failed for req. path '%s'", request_path.c_str())
			return error(reply_buf, http_reply::internal_server_error);
		} // if
#endif // T2H_CONTENT_REQUEST_SYNCED
	}// else

	return base_transport_ev_handler::sent_answ;
}

transport_ev_handler::recv_result 
	transport_ev_handler::on_mhead_request(std::string const & req_path, buffer_type & reply_buf) 
{
	using namespace utility;
	boost::system::error_code ec;
	details::head_reply_param hr_param = { req_path, boost::filesystem::file_size(req_path, ec) };
	if (!ec) {
		details::head_reply reply(reply_buf, hr_param);
		return (!reply.do_formatting_reply()) ?
			error(reply_buf, http_reply::internal_server_error) :
			base_transport_ev_handler::sent_answ;
	}
	return error(reply_buf, http_reply::not_found);
}

transport_ev_handler::recv_result transport_ev_handler::on_mpost_request(
	std::string const & req_path, 
	utility::http_request const & req, 
	buffer_type & reply_buf) 
{
	using namespace utility;
	// TODO impl this funtionality
	return error(reply_buf, http_reply::not_implemented);
}

/**
 * Private helpers
 */
transport_ev_handler::recv_result transport_ev_handler::root_request(buffer_type & reply_buf)  
{
	using namespace utility;
	details::root_reply_param rr_param;
	details::root_reply reply(reply_buf, rr_param);
	return (!reply.do_formatting_reply()) ?
		error(reply_buf, http_reply::internal_server_error) :
		base_transport_ev_handler::sent_answ;
}

bool transport_ev_handler::is_root(std::string const & path) const 
{
	return (path.empty() || path == "/") ?
		true : false;
}

void transport_ev_handler::validate_rheader(utility::range_header & rheader, details::hc_file_info_ptr info) 
{	
	if (rheader.bstart_1 == utility::range_header::all)		
		rheader.bstart_1 = 0;									
	if (rheader.bend_1 == utility::range_header::all)		
		rheader.bend_1 = info->file_size;					

	boost::int64_t const size_for_reading = (1 * (rheader.bend_1 - rheader.bstart_1)) + 1;
	if (size_for_reading > settings_.max_allowed_size_for_reply) {
		boost::int64_t const new_end_offset = rheader.bstart_1 + settings_.max_allowed_size_for_reply; 
		rheader.bend_1 = (new_end_offset >= info->file_size ? info->file_size: new_end_offset);
	}
}

boost::tuple<bool, details::hc_file_info_ptr> 
	transport_ev_handler::sync_request_with_buffer(utility::range_header & rheader, std::string const & req_path) 
{
	using details::hc_file_info_ptr;

	hc_file_info_ptr info = file_info_buffer_.get_info(req_path);
	if (info) {  
		validate_rheader(rheader, info);
		return boost::make_tuple(
					file_info_buffer_.wait_avaliable_bytes(	
						req_path, rheader.bend_1, settings_.max_sync_timeout), 
					info);
	}
	return boost::make_tuple(false, info);
}

} // namespace t2h_core

