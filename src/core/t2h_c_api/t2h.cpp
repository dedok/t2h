#include "t2h.h"
#include "t2h_details.hpp"

#include <boost/thread.hpp>

/**
 * Public t2h_api api
 */

struct underlying_handle {
	boost::mutex lock;
	t2h_core::core_handle_ptr core_handle;
}; 

T2H_STD_API_(t2h_handle_t) t2h_init(char const * config) 
{
	underlying_handle * un_handle = NULL; 
	if (config) 
	{
		t2h_core::core_handle_settings const settings = { config };
		un_handle = new underlying_handle();
		un_handle->core_handle.reset(new t2h_core::core_handle(settings));
		if (!un_handle->core_handle->initialize()) {
			delete un_handle;
			return NULL;
		}
	} // !if
	return (void *)un_handle;
}

T2H_STD_API t2h_close(t2h_handle_t handle) 
{
	underlying_handle * un_handle = (underlying_handle *)handle;
	if (!un_handle) return;
	{ /* Locked scope */
	boost::lock_guard<boost::mutex> guard(un_handle->lock);
	un_handle->core_handle->destroy();
	} /* Locked scope */
	delete un_handle; 
}

T2H_STD_API_(int) t2h_add_torrent(t2h_handle_t handle, char const * path) 
{
	int torrent_id = -1;
	BOOST_ASSERT(handle && path);
	return torrent_id;
}

T2H_STD_API_(int) t2h_add_torrent_url(t2h_handle_t handle, char const * url) 
{
	int torrent_id = -1;
	BOOST_ASSERT(handle && url);
	return torrent_id;
}

T2H_STD_API_(char *) t2h_get_torrent_files(t2h_handle_t handle, int torrent_id) 
{
	BOOST_ASSERT(handle);
	return NULL;
}

T2H_STD_API t2h_start_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
}

T2H_STD_API t2h_paused_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
}

T2H_STD_API t2h_resume_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
}

T2H_STD_API t2h_delete_torrent(t2h_handle_t handle, int torrent_id) 
{
	BOOST_ASSERT(handle);
}

T2H_STD_API t2h_stop_download(t2h_handle_t handle, int torrent_id, int file_id) 
{
	BOOST_ASSERT(handle);
}

