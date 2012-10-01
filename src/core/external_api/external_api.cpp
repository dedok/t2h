#include "t2h.h"
#include "handles_manager.hpp"
#include "external_api_details.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * details/hidden api and types
 */

namespace details {

struct underlying_handle_info {
	underlying_handle_info(std::size_t id_) : id(id_) { }
	std::size_t id;
};

struct underlying_handle 
	: public boost::enable_shared_from_this<underlying_handle> 
{
	t2h_core::core_handle_ptr core_handle;
	std::vector<boost::shared_array<char> > torrents_info_mem;
}; 

typedef boost::shared_ptr<underlying_handle> handle_type;
typedef handles_manager<handle_type> handles_manager_type;

inline static char * string_to_c_string(std::string const & str) 
{
	char * mem = new char[str.size() + 1];
	std::memset(mem, str.size(), 0);
	std::copy(str.begin(), str.end(), mem);
	return mem;
}

} // namespace details

/**
 * Public t2h_api api impl
 */

T2H_STD_API_(t2h_handle_t) t2h_init(char const * config) 
{
	using namespace details;
	if (config) 
	{
		t2h_core::core_handle_settings const settings = { config };
		handle_type handle(new underlying_handle());
		handle->core_handle.reset(new t2h_core::core_handle(settings));
		if (!handle->core_handle->initialize()) 
			return NULL;
		return (void *) new underlying_handle_info(
			handles_manager_type::shared_manager()->registr_new_handle(handle));
	} 
	return NULL;
}

T2H_STD_API t2h_close(t2h_handle_t handle) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (!info) return;
	handles_manager_type::shared_manager()->unregistr_handle(info->id);
	delete info; 
}

T2H_STD_API t2h_wait(t2h_handle_t handle) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info) { 
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		h->core_handle->wait();
	}
}

T2H_STD_API_(T2H_SIZE_TYPE) t2h_add_torrent(t2h_handle_t handle, char const * path) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		return tcore->add_torrent(boost::filesystem::path(path));
	}
	return INVALID_TORRENT_ID;
}

T2H_STD_API_(T2H_SIZE_TYPE) t2h_add_torrent_url(t2h_handle_t handle, char const * url) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		return tcore->add_torrent_url(url);
	}
	return INVALID_TORRENT_ID;
}

T2H_STD_API_(char *) t2h_get_torrent_files(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info && torrent_id != INVALID_TORRENT_ID) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		std::string const info_string = tcore->get_torrent_info(torrent_id);
		char * mem = details::string_to_c_string(info_string);
		h->torrents_info_mem.push_back(boost::shared_array<char>(mem));
		std::cout << mem <<std::endl;
		return mem;
	}
	return NULL;
}

T2H_STD_API_(char *) t2h_start_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id) 
{	
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info && torrent_id != INVALID_TORRENT_ID) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		std::string const path_to_file = tcore->start_torrent_download(torrent_id, file_id);
		return string_to_c_string(path_to_file);
	}
	return NULL;
}

T2H_STD_API t2h_paused_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info && torrent_id != INVALID_TORRENT_ID) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		h->core_handle->get_torrent_core()->pause_download(torrent_id, file_id);
	}
}

T2H_STD_API t2h_resume_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info && torrent_id != INVALID_TORRENT_ID) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		h->core_handle->get_torrent_core()->resume_download(torrent_id, file_id);
	}
}

T2H_STD_API t2h_delete_torrent(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info && torrent_id != INVALID_TORRENT_ID) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		h->core_handle->get_torrent_core()->remove_torrent(torrent_id);
		h->torrents_info_mem.clear();
	}
}

T2H_STD_API t2h_stop_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id) 
{
	using namespace details;
	underlying_handle_info * info = (underlying_handle_info *)handle;
	if (info && torrent_id != INVALID_TORRENT_ID) {
		handle_type h = handles_manager_type::shared_manager()->get_handle(info->id);
		h->core_handle->get_torrent_core()->stop_torrent_download(torrent_id);
	}
}

