#include "external_api_details.hpp"

#include "sequential_torrent_controller.hpp"
#include "syslogger.hpp"

namespace t2h_core {

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

core_handle::core_handle(core_handle_settings const & settings) 
	: settings_(settings), servs_manager_(), sets_manager_() 
{ 
} 

core_handle::~core_handle() 
{
}

bool core_handle::initialize() 
{
	bool state = init_support_system();
	if ((sets_manager_ = setting_manager::shared_manager()) && state) {	
		sets_manager_->load_config(boost::filesystem::path(settings_.config_path));
		if (sets_manager_->config_is_well())
			state = init_core_services();
	} 
	
	if (!state) destroy();
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

bool core_handle::init_support_system() 
{
	try 
	{
		LOG_INIT(details::log_settings)
	} 
	catch (std::exception const &) 
	{
		return false;
	}
	return true;
}

bool core_handle::init_core_services() 
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
	
	return state;
}

torrent_core_ptr core_handle::init_torrent_core() 
{
	torrent_core_ptr tcore;
	torrent_core_params tcore_params;
	try 
	{
		base_torrent_core_cntl_ptr torrent_controller(new sequential_torrent_controller(sets_manager_));
		tcore_params.setting_manager = sets_manager_; 
		tcore_params.controller = torrent_controller;
		tcore.reset(new torrent_core(tcore_params));
		if (!tcore->launch_service())
			return torrent_core_ptr();
	} 
	catch (std::exception const & expt) 
	{
		return torrent_core_ptr(); 
	}
	return tcore;
}

http_server_core_ptr core_handle::init_http_server() 
{
	http_server_core_ptr http_server;
	try 
	{
		http_server.reset(new http_server_core(sets_manager_));
		if (!http_server->launch_service())
			return http_server_core_ptr();
	} 
	catch (std::exception const & expt)
	{
		return http_server_core_ptr();
	}
	return http_server;
}

} // namespace t2h_core

