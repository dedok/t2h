#include "http_transport_ev_handler.hpp"
#include "replies_types.hpp"
#include "http_server_core_config.hpp"

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
	request_parser_(), 
	doc_root_(),
	fp_()
{
	doc_root_ = setting_manager_->get_value<std::string>("doc_root");
	details::t2h_http_fingerprint(fp_);
}

transport_ev_handler::~transport_ev_handler() 
{
}

transport_ev_handler::recv_result transport_ev_handler::on_recv(
	buffer_type const & recv_data, 
	std::size_t recv_data_size, 
	buffer_type & answ_data) 
{
	using namespace utility;
	
	HCORE_TRACE("processing recv data")

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

/** 
 * Private transport_ev_handler api 
 */
boost::tribool transport_ev_handler::parse_recv(
	utility::http_request & request, buffer_type const & data, std::size_t data_size)
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
	
	if (is_root(request_path))  
		return root_request(reply_buf);
	
	request_path = (doc_root_ / request_path).string();
		
	if (!is_valid_path(request_path)) {
		HCORE_WARNING("request_path '%s' not exist ", request_path.c_str())
		return error(reply_buf, http_reply::not_found);
	}

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

	// Follow code should never happens	
	HCORE_WARNING("req.mtype == http_request::munknown for '%s'", req_path.c_str())
	return error(reply_buf, http_reply::bad_request);	
}

transport_ev_handler::recv_result transport_ev_handler::on_mget_request(
	std::string const & request_path, utility::http_request const & req, buffer_type & reply_buf)
{
	using namespace utility;
	
	http_header range_header;	
	bool parse_range_header_result = false;	
	boost::int64_t bytes_begin = 0 , bytes_end = 0;
	
	if (http_request_get_range_header(req, range_header)) { 
		boost::tie(bytes_begin, bytes_end, parse_range_header_result) 
			= parse_range_header(range_header);
		if (!parse_range_header_result) { 
			HCORE_WARNING(
				"ill formed data, parsing of the 'Range' header failed, req. bytes: s '%i' e '%i'"
				, (int)bytes_begin, (int)bytes_end)
			return error(reply_buf, http_reply::bad_request);
		}
	} else // http_request_get_range_header
		boost::tie(bytes_begin, bytes_end) = get_file_size_range(request_path);
	
	details::partial_content_reply_param const pcr_param = 
		details::create_partial_content_param(request_path, boost::filesystem::file_size(request_path), bytes_begin, bytes_end);
	details::partial_content_reply pcr_reply(reply_buf, pcr_param);

	if (!pcr_reply.do_formatting_reply()) {
		HCORE_WARNING("preparing reply failed for req. path '%s'", request_path.c_str())
		return error(reply_buf, http_reply::internal_server_error);
	}

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

bool transport_ev_handler::is_valid_path(std::string const & path) const
{
	boost::system::error_code error;
	// TODO add extra test(getting data from notification center) + exclude all request which not in the root
	if (boost::filesystem::exists(path, error))
		return (!error ? true : false);
	return false;
}

boost::tuple<boost::int64_t, boost::int64_t> 
	transport_ev_handler::get_file_size_range(std::string const & path) const 
{
	boost::int64_t first = 0, last = 0;
	// TODO replce getting file size from FS to getting file size from notification center
	last = boost::filesystem::file_size(path);
	return boost::make_tuple(first, last);
}

} // namespace t2h_core

