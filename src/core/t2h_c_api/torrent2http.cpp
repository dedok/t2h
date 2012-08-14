#include "torrent2http.h"

#include "syslogger.hpp"
#include "services_manager.hpp"
#include "t2h_torrent_cntl.hpp"
#include "t2h_http_server_cntl.hpp"

#include <boost/thread.hpp>

/**
 * Private hidden t2h_core api and details
 */

struct t2h_handle {
	boost::mutex lock;	
	common::services_manager servs_manager;
	t2h_core::setting_manager_ptr sets_manager;
};

typedef t2h_handle * t2h_handle_ptr;

namespace t2h_core {
	inline bool init_shared_setting_manager(t2h_handle_ptr handle, char const * config) 
	{
		t2h_core::setting_manager_ptr sets_manager
			= t2h_core::setting_manager::shared_manager();	
		if (sets_manager) {
			sets_manager->load_config(boost::filesystem::path(config));
			return (handle->sets_manager = sets_manager)->config_is_well();
		}
		return false;
	}

	inline bool init_syslogger(t2h_handle_ptr handle) 
	{
		static syslogger_settings const log_settings = {
			"com.t2h.HttpServer", 
			"application",
			"/Users/dedokOne/application.log"
		};

		LOG_INIT(log_settings)
		
		return true;
	}

	inline bool init_and_run_services(t2h_handle_ptr handle) 
	{
		bool state = false;
		common::services_manager & services_manager = handle->servs_manager;
		common::base_service_ptr http_server(new t2h_core::http_server_cntl("t2h_http_server", handle->sets_manager));
		state = http_server->launch_service();
		services_manager.registrate(http_server);
		return state;
	}

} // namespace t2h_core

/**
 * Public t2h_api api
 */

T2H_STD_API_(t2h_handle_t) t2h_init(char const * config) 
{
	t2h_handle * handle = NULL; 
	if (!config) 
		return handle;

	if ((handle = new (std::nothrow) t2h_handle) != NULL) {
		if (!t2h_core::init_shared_setting_manager(handle, config))
			goto error_exit;
		if (!t2h_core::init_syslogger(handle))
			goto error_exit;
		if (!t2h_core::init_and_run_services(handle))
			goto error_exit;
	} 

	return handle;

error_exit :
	delete handle;
	return NULL;
}

T2H_STD_API t2h_close(t2h_handle_t handle) 
{
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		{ /* Locked scope */
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
		handle_ptr->servs_manager.stop_all();
		} /* Locked scope */
		delete handle_ptr; handle_ptr = NULL;
	}
}

T2H_STD_API_(int) t2h_add_torrent(t2h_handle_t handle, char const * path) 
{
	int torrent_id = -1;
	BOOST_ASSERT(handle && path);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr && path) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
		
	}
	return torrent_id;
}

T2H_STD_API_(int) t2h_add_torrent_url(t2h_handle_t handle, char const * url) 
{
	int torrent_id = -1;
	BOOST_ASSERT(handle && url);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr && url) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
	return torrent_id;
}

T2H_STD_API_(char *) t2h_get_torrent_files(t2h_handle_t handle, int torrent_id) 
{
	BOOST_ASSERT(handle);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
	return NULL;
}

T2H_STD_API t2h_start_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
}

T2H_STD_API t2h_paused_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
}

T2H_STD_API t2h_resume_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
}

T2H_STD_API t2h_delete_torrent(t2h_handle_t handle, int torrent_id) 
{
	BOOST_ASSERT(handle);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
}

T2H_STD_API t2h_stop_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
	t2h_handle_ptr handle_ptr = (t2h_handle *)handle;
	if (handle_ptr) {
		boost::lock_guard<boost::mutex> guard(handle_ptr->lock);
	}
}

