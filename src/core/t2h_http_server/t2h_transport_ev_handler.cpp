#include "t2h_transport_ev_handler.hpp"
#include "http_utility.hpp"
#include "syslogger.hpp"

/** 
 * Public transport_ev_handler api 
 */

namespace t2h_core {

transport_ev_handler::transport_ev_handler(setting_manager_ptr setting_manager) 
	: base_transport_ev_handler(), setting_manager_(setting_manager) , request_parser_(), doc_root_() 
{
	doc_root_ = setting_manager_->get_value<std::string>("doc_root");
}

transport_ev_handler::~transport_ev_handler() 
{
}

transport_ev_handler::recv_result transport_ev_handler::on_recv(
		buffer_type const & recv_data, std::size_t recv_data_size, buffer_type & answ_data) 
{
	LOG_TRACE("processing recv data")

	using namespace utility;

	http_reply serv_reply(answ_data);
	http_request client_request;

	boost::tribool result = parse_recv(client_request, recv_data, recv_data_size);
	if (result) 
		return on_recv_succ(client_request, serv_reply);
	else if (!result) {
		LOG_WARNING("parsing of the recv data failed")
		return on_recv_error(serv_reply, http_reply::bad_request);
	}
	return on_recv_more_data(serv_reply);
}

void transport_ev_handler::on_error(int error) 
{
	LOG_WARNING("error code '%i'", error)
}

void transport_ev_handler::on_close() 
{
	LOG_TRACE("connection closed")
}

transport_ev_handler::ptr_type transport_ev_handler::clone() 
{
	return common::base_transport_ev_handler_ptr(
		new transport_ev_handler(setting_manager_));
}

/** 
 * Private transport_ev_handler api 
 */
boost::tribool transport_ev_handler::parse_recv(
	utility::http_request & request, 
	buffer_type const & data, 
	std::size_t data_size)
{
	boost::tribool result;
	request_parser_.reset_state();
	boost::tie(result, boost::tuples::ignore) 
		= request_parser_.parse(request, data.data(), data.data() + data_size);
	return result;
}

transport_ev_handler::recv_result transport_ev_handler::on_recv_error(
	utility::http_reply & serv_reply, utility::http_reply::status_type status) 
{
	using namespace utility;
	serv_reply.stock_reply(status);
	return base_transport_ev_handler::bad_data;
}

transport_ev_handler::recv_result transport_ev_handler::on_recv_succ(
	utility::http_request const & req, utility::http_reply & serv_reply) 
{
	using namespace utility;
	
	http_header range_header;
	std::string request_path;
	bool parse_range_header_result = false;	
	boost::int64_t bytes_begin = 0 , bytes_end = 0;
	
	if (!url_decode(req.uri, request_path)) {
		LOG_WARNING("uri decode failed with data '%s'", req.uri.c_str())
		return on_recv_error(serv_reply, http_reply::bad_request);
	}

	request_path = (doc_root_ / request_path).string();
	
	if (!http_request_get_range_header(req, range_header)) {
		LOG_WARNING("ill formed data, getting of the range header failed")
		return on_recv_error(serv_reply, http_reply::bad_request);
	}

	boost::tie(bytes_begin, bytes_end, parse_range_header_result) 
		= parse_range_header(range_header);
	if (!parse_range_header_result) { 
		LOG_WARNING(
			"ill formed data, parsing of the 'Range' header failed, req. bytes: s '%i' e '%i'"
			, bytes_begin, bytes_end)
		return on_recv_error(serv_reply, http_reply::bad_request);
	}
	
	http_reply::formating_result const state = 
		serv_reply.format_partial_content(request_path, bytes_begin, bytes_end);
	switch (state)
	{
		case http_reply::formating_succ :
			break;
		break;
		case http_reply::file_not_exist :
			return on_recv_error(serv_reply, http_reply::not_found);
		break;
		case http_reply::io_error : 
		case http_reply::buffer_error : 
		case http_reply::unknown_error :
			LOG_WARNING("get content data failed, with state '%i', req. path '%s', req. bytes: s '%i' e '%i'", 
				(int)state, request_path.c_str(), bytes_begin, bytes_end)
			return on_recv_error(serv_reply, http_reply::internal_server_error);
		default : /* Shoud never happen */ 
			return on_recv_error(serv_reply, http_reply::not_implemented);
	} // !switch

	return base_transport_ev_handler::sent_answ;
}

transport_ev_handler::recv_result transport_ev_handler::on_recv_more_data(
	utility::http_reply & serv_reply) 
{
	using namespace utility; 
	return base_transport_ev_handler::more_data;         
}

} // namespace t2h_core

