#ifndef T2H_HTTP_SERVER_CNTL_HPP_INCLUDED
#define T2H_HTTP_SERVER_CNTL_HPP_INCLUDED

#include "t2h_settings_manager.hpp"
#include "base_transport.hpp"
#include "base_transport_ev_handler.hpp"

#include "base_service.hpp"

namespace t2h_core {

class http_server_cntl 
	: public common::base_service 
{
public :
	http_server_cntl(std::string const & service_name, setting_manager_ptr setting_manager);
	~http_server_cntl();
	
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
	
	virtual common::base_service::ptr_type clone();

private :
	common::base_transport_ptr transport_;
	common::base_transport_ev_handler_ptr transport_ev_handler_;
	setting_manager_ptr setting_manager_;

};

}

#endif

