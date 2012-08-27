#ifndef HTTP_SERVER_CORE_HPP_INCLUDED
#define HTTP_SERVER_CORE_HPP_INCLUDED

#include "setting_manager.hpp"
#include "base_transport.hpp"
#include "base_transport_ev_handler.hpp"

#include "base_service.hpp"

namespace t2h_core {

class http_server_core : public common::base_service {
public :
	static char const * http_core_service_name;
	
	explicit http_server_core(setting_manager_ptr setting_manager);
	~http_server_core();
	
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
	
	virtual common::base_service::ptr_type clone();

	virtual common::base_service::service_state get_service_state() const;

private :
	common::base_transport_ptr transport_;
	common::base_transport_ev_handler_ptr transport_ev_handler_;
	setting_manager_ptr setting_manager_;
	common::base_service::service_state volatile mutable cur_state_;

};

}

#endif

