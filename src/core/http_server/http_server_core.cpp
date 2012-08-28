#include "http_server_core.hpp"

#include "http_transport_ev_handler.hpp"
#include "transport_types.hpp"
#include "http_server_core_config.hpp"

namespace t2h_core {

/**
 * Privite hidden helpers
 */

inline static common::transport_config from_setting_manager(	
	setting_manager_ptr setting_manager) 
{
	common::transport_config const config = { 
		setting_manager->get_value<std::string>("server_addr"),
		setting_manager->get_value<std::string>("server_port"),
		setting_manager->get_value<std::size_t>("workers")
	};
	return config;
}

/**
 * Public http_server_core api
 */

char const * http_server_core::http_core_service_name = "t2h_http_core";

http_server_core::http_server_core(setting_manager_ptr setting_manager) 
	: base_service(http_core_service_name), 
	transport_(),
	transport_ev_handler_(), 
	setting_manager_(setting_manager),
	cur_state_()
{ 
}  

http_server_core::~http_server_core() 
{

}

bool http_server_core::launch_service() 
{
	using namespace common;	
	try 
	{
		if (cur_state_ == base_service::service_running) 
			return false;
		
		transport_config const tr_config = from_setting_manager(setting_manager_);
			
		transport_ev_handler_.reset(new transport_ev_handler(setting_manager_));
		transport_.reset(new asio_socket_transport(tr_config, transport_ev_handler_));
		
		transport_->initialize();	
		transport_->establish_connection();

		cur_state_ = base_service::service_running;
	} 
	catch (common::transport_exception const & expt) 
	{
		HCORE_ERROR("transport init/run failed, with message '%s'", expt.what())
		return false;
	}
	return (cur_state_ == base_service::service_running);
}

void http_server_core::stop_service() 
{
	using namespace common;
	try 
	{
		if (cur_state_ == base_service::service_running) {
			transport_->stop_connection();	
			cur_state_ = base_service::service_stoped;
		}
	}
	catch (transport_exception const & expt) 
	{
		HCORE_ERROR("transport stop failed, with message '%s'", expt.what())
	}
}

void http_server_core::wait_service() 
{
	using namespace common;
	try 
	{
		if (cur_state_ == base_service::service_running)
			transport_->stop_connection();
	} 
	catch (transport_exception const & expt) 
	{
		HCORE_ERROR("transport wait failed, with message '%s'", expt.what())
	}
}

common::base_service_ptr http_server_core::clone() 
{
	return boost::shared_ptr<http_server_core>(
		new http_server_core(setting_manager_));
}

common::base_service::service_state http_server_core::get_service_state() const 
{
	return cur_state_;
}

/**
 * Private http_server_core api
 */

} // namespace t2h_core

