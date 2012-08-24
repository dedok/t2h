#include "t2h_details.hpp"

#include "serial_download_tcore_cntl.hpp"
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
	try 
	{
		base_torrent_core_cntl_ptr serial_download_cntl(new serial_download_tcore_cntl(sets_manager_));
		torrent_core_params tcore_params;
		tcore_params.setting_manager = sets_manager_; 
		tcore_params.controller = serial_download_cntl;
		torrent_core_ptr tcore(new torrent_core(tcore_params));
		if (tcore->launch_service())
			state = servs_manager_.registrate(tcore);
	}
	catch (std::exception const & expt) 
	{
		return false;
	}
	return state;
}


} // namespace t2h_core

