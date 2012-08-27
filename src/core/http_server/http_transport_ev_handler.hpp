#ifndef HTTP_TRANSPORT_EV_HANDLER_HPP_INCLUDED
#define HTTP_TRANSPORT_EV_HANDLER_HPP_INCLUDED

#include "http_utility.hpp"
#include "setting_manager.hpp"
#include "base_transport_ev_handler.hpp"

#include <boost/tuple/tuple.hpp>
#include <boost/logic/tribool.hpp>

namespace t2h_core {

class transport_ev_handler : public common::base_transport_ev_handler {
public :
	explicit transport_ev_handler(setting_manager_ptr setting_manager);
	virtual ~transport_ev_handler();

	virtual recv_result on_recv(buffer_type const & recv_data, std::size_t recv_data_size, buffer_type & answ_data);
	virtual void on_error(int error_code);	
	virtual void on_close();

	virtual ptr_type clone();

private :	
	boost::tribool parse_recv(utility::http_request & request, 
					buffer_type const & data, std::size_t data_size);
	
	recv_result on_recv_succ(utility::http_request const & req, utility::http_reply & serv_reply);
	recv_result on_recv_error(utility::http_reply & serv_reply, utility::http_reply::status_type status);
	recv_result on_recv_more_data(utility::http_reply & answ_data);

	utility::http_request_parser request_parser_;
	setting_manager_ptr setting_manager_;
	boost::filesystem::path doc_root_;

};

} // namespace t2h_core

#endif

