#include "t2h.h"
#include "setting_manager.hpp"
#include "handles_manager.hpp"
#include "external_api_details.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_array.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * details/hidden api and types
 */

#if (defined(WIN32) || defined(WIN64)) && defined(T2H_EXPORT)
#	define T2H_SHARED_EXPORT_FUNCDNAME comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
#else
#	define T2H_SHARED_EXPORT_FUNCDNAME
#endif // T2H_SHARED_EXPORT_FUNCDNAME

#define T2H_PASSED_HANDLES_CHECK

#define T2H_RETURN_IF(x, x1) if (x) { x1 };

namespace details {

/**
 * Extra torrent information
 */
struct underlying_info {
	typedef std::list<boost::shared_array<char> > mem_collector_type;
	mem_collector_type mem_collector;
	T2H_TORRENT_ID_TYPE_ tid;
};

typedef boost::shared_ptr<underlying_info> underlying_info_ptr;

/**
 * t2h hidden underlying handle type
 */
struct underlying_handle 
	: public boost::enable_shared_from_this<underlying_handle> 
{
	typedef std::map<T2H_SIZE_TYPE, underlying_info_ptr> torrents_map_type;	
	t2h_core::core_handle_ptr core_handle;
	torrents_map_type torrents;
	boost::mutex mutable torrents_lock;

/**
 * Public underlying_handle api 
 */
	inline T2H_SIZE_TYPE add_info(underlying_info const & info) 
	{
		//TODO Add element duplicate test
		boost::lock_guard<boost::mutex> guard(torrents_lock);
		underlying_info_ptr ui_ptr(new underlying_info());
		ui_ptr->tid = info.tid;
		ui_ptr->mem_collector = info.mem_collector;
#if !defined(T2H_INT_WORKAROUND)
		torrents.insert(std::make_pair(info.tid, ui_ptr));
		return info.tid;
#else
		torrents_map_type::size_type const fake_id = torrents.size() + 1;
		torrents.insert(std::make_pair(fake_id, ui_ptr));
		return (T2H_SIZE_TYPE)fake_id;
#endif // T2H_INT_WORKAROUND
	}
	
	inline boost::tuple<underlying_info_ptr, bool> get_info(T2H_SIZE_TYPE tid) const 
	{
		boost::lock_guard<boost::mutex> guard(torrents_lock);
		torrents_map_type::const_iterator found = torrents.find(tid);
		if (found != torrents.end()) 
			return boost::make_tuple(found->second, true);
		return boost::make_tuple(underlying_info_ptr(), false);
	}

	inline void remove_info(T2H_SIZE_TYPE tor_id) 
	{
		boost::lock_guard<boost::mutex> guard(torrents_lock);
		torrents_map_type::iterator found = torrents.find(tor_id);
		if (found != torrents.end()) 
			torrents.erase(found);
	}

}; 

typedef boost::shared_ptr<underlying_handle> handle_type;
typedef handles_manager<handle_type> handles_manager_type;

static inline char * string_to_c_string(std::string const & str) 
{
	char * c_str = NULL;
	if (!str.empty()) {
		c_str = new char[str.size() + 1]; 
		std::copy(str.begin(), str.end(), c_str);
		c_str[str.size()] = '\0';
	}
	return c_str;
}

static inline std::string create_url(
	t2h_core::setting_manager_ptr sets_manager, std::string const & file_path) 
{
	static std::string const http_protocol_prefix = "http://";
	if (file_path.empty()) return std::string();
	std::string well_formed_fp = file_path;
#if defined(WIN32) || defined(WIN64) || defined (__CYGWIN__)
	std::replace_if(well_formed_fp.begin(), well_formed_fp.end(), 
		boost::lambda::_1 == '\\', '/');
#endif // WINXX
	return std::string(http_protocol_prefix + 
		sets_manager->get_value<std::string>("server_addr") + 
		+ ":" + sets_manager->get_value<std::string>("server_port") + 
		"/" + well_formed_fp);
}

static inline t2h_handle_t t2h_init_(char const * config, bool load_from_file) 
{
	t2h_handle_t handle_t = INVALID_T2H_HANDLE;
	if (config) {
		t2h_core::core_handle_settings const settings = { config, load_from_file };
		handle_type handle(new underlying_handle());
		handle->core_handle.reset(new t2h_core::core_handle(settings));
		if (handle->core_handle->initialize()) 
			handle_t = handles_manager_type::shared_manager()->registr_new_handle(handle);
	} 
	return handle_t;
}

} // namespace details

/**
 * Public t2h_api api impl
 */

T2H_STD_API_(t2h_handle_t) t2h_init(char const * config) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	return details::t2h_init_(config, false);
}

T2H_STD_API_(t2h_handle_t) t2h_init_2(char const * file_path) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	return details::t2h_init_(file_path, true);
}

T2H_STD_API t2h_close(t2h_handle_t handle) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;
	if (handle > INVALID_T2H_HANDLE) {
		handle_type h = handles_manager_type::shared_manager()->unregistr_handle(handle);
		h->core_handle->destroy();
	}
}

T2H_STD_API t2h_wait(t2h_handle_t handle) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;
	if (handle > INVALID_T2H_HANDLE) { 
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		h->core_handle->wait();
	}
}

// TODO Think about case when underlying_handle::add_info failed
T2H_STD_API_(T2H_SIZE_TYPE) t2h_add_torrent(t2h_handle_t handle, char const * path) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;
	if (handle > INVALID_T2H_HANDLE && path) {
		underlying_info handle_info;
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		if ((handle_info.tid = tcore->add_torrent(boost::filesystem::path(path))) != 
			t2h_core::torrent_core::invalid_torrent_id) 
		{
			return h->add_info(handle_info);
		}
	}
	return INVALID_TORRENT_ID;
}

// TODO Think about case when underlying_handle::add_info failed
T2H_STD_API_(T2H_SIZE_TYPE) t2h_add_torrent_url(t2h_handle_t handle, char const * url) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;
	if (handle > INVALID_T2H_HANDLE && url) {
		underlying_info handle_info;
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		if ((handle_info.tid = tcore->add_torrent_url(url)) != 
			t2h_core::torrent_core::invalid_torrent_id) 
		{
			return h->add_info(handle_info);
		}
	}
	return INVALID_TORRENT_ID;
}

T2H_STD_API_(char *) t2h_get_torrent_files(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;

	char * mem = NULL;
	if (handle > INVALID_T2H_HANDLE && torrent_id != INVALID_TORRENT_ID) 
	{
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		
		underlying_info_ptr info; bool result = false; 
		boost::tie(info, result) = h->get_info(torrent_id); 	
		if (result) { 
			std::string const info_string = tcore->get_torrent_info(info->tid);
			if ((mem = details::string_to_c_string(info_string)) != NULL) 
				info->mem_collector.push_back(boost::shared_array<char>(mem));
		} // result
	}
	return mem;
}

T2H_STD_API_(char *) t2h_start_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id) 
{	
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	// TODO add server addres resolving(auto, manual)
	using namespace details;

	char * mem = NULL;
	if (handle > INVALID_T2H_HANDLE && torrent_id != INVALID_TORRENT_ID && file_id >= 0) 
	{
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		t2h_core::torrent_core_ptr tcore = h->core_handle->get_torrent_core();
		
		underlying_info_ptr info; bool result = false; 
		boost::tie(info, result) = h->get_info(torrent_id);
		if (result) { 
			std::string const url = details::create_url(h->core_handle->get_setting_manager(), 
										tcore->start_torrent_download(info->tid, file_id));
			if ((mem = details::string_to_c_string(url)) != NULL) 
				info->mem_collector.push_back(boost::shared_array<char>(mem));
		} // result
	}
	return mem;
}

T2H_STD_API t2h_paused_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;
	
	underlying_info_ptr info; bool result = false; 
	if (handle > INVALID_T2H_HANDLE && torrent_id != INVALID_TORRENT_ID && file_id >= 0) 
	{
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);	
		boost::tie(info, result) = h->get_info(torrent_id); 	
		if (result)
			h->core_handle->get_torrent_core()->pause_download(info->tid, file_id);
	}
}

T2H_STD_API t2h_resume_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;

	underlying_info_ptr info; bool result = false; 
	if (handle > INVALID_T2H_HANDLE && torrent_id != INVALID_TORRENT_ID && file_id >= 0) 
	{
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		boost::tie(info, result) = h->get_info(torrent_id); 	
		if (result)
			h->core_handle->get_torrent_core()->resume_download(info->tid, file_id);
	}
}

T2H_STD_API t2h_delete_torrent(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;
	
	underlying_info_ptr info; bool result = false; 
	if (handle > INVALID_T2H_HANDLE && torrent_id != INVALID_TORRENT_ID) 
	{
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		boost::tie(info, result) = h->get_info(torrent_id); 	
		if (result) {
			h->core_handle->get_torrent_core()->remove_torrent(info->tid);
#if defined(T2H_INT_WORKAROUND)
			h->remove_info(torrent_id);
#else 
			h->remove_info(info->tid);
#endif // T2H_INT_WORKAROUND
		} // result
	}
}

T2H_STD_API t2h_stop_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id) 
{
#pragma T2H_SHARED_EXPORT_FUNCDNAME
	using namespace details;

	underlying_info_ptr info; bool result = false; 
	if (handle > INVALID_T2H_HANDLE && torrent_id != INVALID_TORRENT_ID) 
	{
		handle_type h = handles_manager_type::shared_manager()->get_handle(handle);
		boost::tie(info, result) = h->get_info(torrent_id); 	
		if (result)
			h->core_handle->get_torrent_core()->stop_torrent_download(info->tid);
	}
}

#undef T2H_PASSED_HANDLES_CHECK
#undef T2H_RETURN_IF
#undef T2H_SHARED_EXPORT_FUNCDNAME

