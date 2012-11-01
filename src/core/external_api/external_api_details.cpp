#include "external_api_details.hpp"

#include "syslogger.hpp"
#include "hc_event_source_adapter.hpp"
#include "sequential_torrent_controller.hpp"

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101) 
#endif

namespace t2h_core {

/**
 * Private-hidden core_handle helpers
 */

namespace details {

char const * core_handle_component_name = "com.t2h.core";
char const * core_api_id = "t2h_api_";
char const * syslog_path = "";

syslogger_settings const log_settings = {
	core_handle_component_name, 
	core_api_id,
	syslog_path
};

} // namespace details

/**
 * Public core_handle api
 */

core_handle::core_handle(core_handle_settings const & settings) 
	: settings_(settings), servs_manager_(), sets_manager_() 
{ 
} 

core_handle::~core_handle() 
{
}

bool core_handle::initialize() 
{
	bool state = false;
	try 
	{
		if ((sets_manager_ = setting_manager::shared_manager())) 
		{
			init_support_system();
			if (settings_.config_load_from_file) 
				sets_manager_->load_config(boost::filesystem::path(settings_.config));
			else
				sets_manager_->init_config(settings_.config);

			if ((state = sets_manager_->config_is_well()))
				init_core_services();
		} 
	} 
	catch (std::exception const & expt) 
	{ 
		return false; 
	}
	
	return state;
}

void core_handle::destroy() 
{
	servs_manager_.stop_all();
	sets_manager_.reset();
}

void core_handle::wait() 
{
	servs_manager_.wait_all();
}

/**
 * Private core_handle api
 */

void core_handle::init_support_system() 
{
	LOG_INIT(details::log_settings)
}

void core_handle::init_core_services() 
{
	bool state = false;
	torrent_core_ptr torrent_core;
	http_server_core_ptr http_server;
	
	if ((torrent_core = init_torrent_core())) { 
		if ((http_server = init_http_server())) {
			state = servs_manager_.registrate(torrent_core);
			state = servs_manager_.registrate(http_server);
		} // !if		
	}

	if (!state) {
		torrent_core->stop_service();
		http_server->stop_service();
	} 
}

torrent_core_ptr core_handle::init_torrent_core() 
{
	torrent_core_ptr tcore;
	torrent_core_params tcore_params;
	try 
	{
		tcore_params.setting_manager = sets_manager_; 
		tcore_params.controller.reset(new sequential_torrent_controller());
		tcore_params.event_handler.reset(new details::hc_event_source_adapter());
		
		tcore.reset(new torrent_core(tcore_params));
		return (!tcore->launch_service()) ? torrent_core_ptr() : tcore;
	} 
	catch (std::exception const & expt) { }
	return torrent_core_ptr();
}

http_server_core_ptr core_handle::init_http_server() 
{
	http_server_core_ptr http_server;
	try 
	{
		http_server.reset(new http_server_core(sets_manager_));
		return (!http_server->launch_service()) ? http_server_core_ptr() : http_server;
	} 
	catch (std::exception const & expt){ }
	return http_server;
}

} // namespace t2h_core

#if defined(WIN32)
#	pragma warning(pop)
#endif

