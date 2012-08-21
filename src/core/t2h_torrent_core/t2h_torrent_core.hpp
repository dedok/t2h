#ifndef T2H_TORRENT_CORE_HPP_INCLUDED
#define T2H_TORRENT_CORE_HPP_INCLUDED

#include "base_service.hpp"
#include "t2h_torrent_core_config.hpp"
#include "t2h_settings_manager.hpp"
#include "base_torrent_core_cntl.hpp"

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem/path.hpp>

namespace t2h_core {

namespace details {

/** torrent_core_settings is main settings for the t2h_core::torrent_core,
 	for better abstraction this object is hidden and for public access, 
	use t2h_core::setting_manager_ptr which passed via ctor of t2h_core::torrent_core 
	in t2h_core::torrent_core_params */
struct torrent_core_settings {
	std::string save_root;
	int port_start;
	int port_end;
	int max_alert_wait_time;
};

enum torrent_status {
	in_process = 0x1,
	request_to_stop,
	request_to_pause,
	request_to_resume,
	unknown_request = request_to_resume + 1
};

struct torrent_handle_ex {
	typedef boost::hash<std::string> hash_func_type;
	std::string path;
	torrent_status status;
	libtorrent::torrent_handle lt_handle;
	std::size_t hash_path;
};

typedef std::map<std::size_t, torrent_handle_ex> torrents_list_type;
typedef torrents_list_type::const_iterator tl_const_iteretator;
typedef torrents_list_type::iterator tl_iterator;

struct torrents_list_ex {
	torrents_list_type list;
	boost::mutex lock;
};


} // namespace details

/** The main torrent_core parameters, all filds of the t2h_core::torrent_core_params 
	must set to valid values, otherwise SEGFAULT ... */ 
struct torrent_core_params {
	setting_manager_ptr setting_manager;
	base_torrent_core_cntl_ptr controller;
};

/**
 * 
 */
class torrent_core : public common::base_service {
public :
	enum { invalid_torrent_id = -1 };

	torrent_core(torrent_core_params const & params, std::string const & name);
	~torrent_core();
		
	/** Outside & manadgable via service_manager interface, not thread safe */
	
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
		
	virtual ptr_type clone();
	
	/** Outside control interface, not thread safe */
	
	void set_controller(base_torrent_core_cntl_ptr controller);
	
	boost::tuple<int, boost::filesystem::path> add_torrent(boost::filesystem::path const & path);
	void pause_torrent(std::size_t torrent_id);
	void resume_download(std::size_t torrent_id);	
	void remove_torrent(std::size_t torrent_id);
	void stop_download(std::size_t torrent_id);
	
	inline details::torrents_list_ex & get_torrents_list() 
	{ 
		return torrents_list_; 
	}
	
	inline details::torrents_list_ex const & get_torrents_list() const 
	{ 
		return torrents_list_; 
	}

private :

	bool init_core_session();
	void setup_core_session();
	bool init_torrent_core_settings();
	
	void core_main_loop();
	void handle_core_notifications();	
	bool is_critical_error(libtorrent::alert * alert);
	void handle_critical_error_notification(libtorrent::alert * alert);
	
	bool prepare_torrent_params(
		libtorrent::add_torrent_params & torrent_params, boost::filesystem::path const & path);
	bool prepare_torrent_sandbox(libtorrent::add_torrent_params & torrent_params);
	void stop_download_unsafe(std::size_t torrent_id);
	
	torrent_core_params mutable params_;			
	bool volatile is_running_;						
	details::torrent_core_settings settings_;		
	boost::scoped_ptr<boost::thread> thread_loop_;
	details::torrents_list_ex torrents_list_;
	libtorrent::session core_session_;

};

} // namespace t2h_core

#endif

