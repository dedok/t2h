#ifndef HTTP_TRANSPORT_EV_HANDLER_HPP_INCLUDED
#define HTTP_TRANSPORT_EV_HANDLER_HPP_INCLUDED

#include "http_utility.hpp"
#include "setting_manager.hpp"
#include "base_transport_ev_handler.hpp"

#include <boost/tuple/tuple.hpp>
#include <boost/logic/tribool.hpp>

namespace t2h_core {

/**
 * transport_ev_handler
 */
class transport_ev_handler : public common::base_transport_ev_handler {
public :
	explicit transport_ev_handler(setting_manager_ptr setting_manager);
	virtual ~transport_ev_handler();

	/* recv callbacks ifaces impl*/
	virtual recv_result on_recv(buffer_type const & recv_data, std::size_t recv_data_size, buffer_type & answ_data);
	virtual void on_error(int error_code);	
	virtual void on_close();

	/* hidden copy semantic */
	virtual ptr_type clone();

private :	
	/* data workers */
	boost::tribool parse_recv(utility::http_request & request, 
					buffer_type const & data, std::size_t data_size);
	
	recv_result proceed_execute_data(utility::http_request const & req, buffer_type & serv_reply);
	recv_result more_data(buffer_type & answ_data);
	recv_result error(buffer_type & serv_reply, utility::http_reply::status_type status);
	
	/* request(s) dipatching */
	recv_result dispatch_client_request(
		std::string const & req_path, utility::http_request const & req, buffer_type & serv_reply); 
	recv_result on_mpost_request(
		std::string const & req_path, utility::http_request const & req, buffer_type & serv_reply);
	recv_result on_mhead_request(std::string const & req_path, buffer_type & serv_reply);
	recv_result on_mget_request(
		std::string const & req_path, utility::http_request const & req, buffer_type & serv_reply);
	recv_result root_request(buffer_type & serv_reply);
	
	/* helpers */
	bool is_root(std::string const & path) const; 
	bool is_valid_path(std::string const & path) const; 
	boost::tuple<boost::int64_t, boost::int64_t> get_file_size_range(std::string const & path) const;

	utility::http_request_parser request_parser_;
	setting_manager_ptr setting_manager_;
	boost::filesystem::path doc_root_;
	utility::fingerprint fp_;

};

} // namespace t2h_core

#endif

