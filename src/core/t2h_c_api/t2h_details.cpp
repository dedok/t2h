#include "t2h_details.hpp"

#include "syslogger.hpp"
#include "t2h_torrent_core.hpp"
#include "t2h_http_server_cntl.hpp"

namespace t2h_core {

namespace details {

const char const * core_handle_component_name = "com.t2h.core";
const char const * bar_core = "t2h_api";
const char const * syslog_path = "";

syslogger_settings const log_settings = {
	details::core_handle_component_name, 
	details::bar_core,
	details::syslog_path
};

} // namespace details

core_handle::core_handle(core_handle_settings const & settings) 
	: settings_(settings), servs_manager_(), sets_manager_() 
{ 
} 

core_handle::~core_handle() 
{
}

bool core_handle::initialization() 
{
	bool state = init_support_system();
	if (sets_manager_ = t2h_core::setting_manager::shared_manager() && state) {	
		sets_manager->load_config(boost::filesystem::path(settings_.config_path));
		if (state = sets_manager->config_is_well())
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
	return state;
}


} // namespace t2g_core

