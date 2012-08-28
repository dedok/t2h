#ifndef TORRENT_CORE_HPP_INCLUDED
#define TORRENT_CORE_HPP_INCLUDED

#include "base_service.hpp"
#include "setting_manager.hpp"
#include "torrent_core_config.hpp"
#include "base_torrent_core_cntl.hpp"

#include <map>

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include <boost/thread.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/enable_shared_from_this.hpp>

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
	static const char * this_service_name;

	torrent_core(torrent_core_params const & params);
	~torrent_core();
		
	/** Outside & manadgable via service_manager interface, not thread safe */
	
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
		
	virtual ptr_type clone();
	
	virtual common::base_service::service_state get_service_state() const;	
	
	/** Outside control interface, not thread safe */
	
	void set_controller(base_torrent_core_cntl_ptr controller);
	
	int add_torrent(boost::filesystem::path const & path);
	int add_torrent_url(std::string const & url);
	
	std::string get_torrent_info(int torrent_id) const;

	std::string start_torrent_download(int torrent_id, int file_id);
	void pause_download(int torrent_id, int file_id);
	void resume_download(int torrent_id, int file_id);	
	void remove_torrent(int torrent_id);
	void stop_torrent_download(int torrent_id);
	
private :
	bool init_core_session();
	void setup_core_session();
	bool init_torrent_core_settings();
	
	void core_main_loop();
	void handle_core_notifications();	
	bool is_critical_error(libtorrent::alert * alert);
	void handle_critical_error_notification(libtorrent::alert * alert);

	bool prepare_torrent_params_for_file(
		libtorrent::add_torrent_params & torrent_params, boost::filesystem::path const & path);
	void prepare_torrent_params_for_url(
		libtorrent::add_torrent_params & torrent_params, std::string const & path);
	bool prepare_torrent_sandbox(libtorrent::add_torrent_params & torrent_params);
	
	torrent_core_params mutable params_;			
	base_service::service_state volatile mutable cur_state_;
	details::torrent_core_settings settings_;	
	details::torrents_map_type torrents_map_;
	boost::scoped_ptr<boost::thread> thread_loop_;
	libtorrent::session core_session_;
};

typedef boost::shared_ptr<torrent_core> torrent_core_ptr;

} // namespace t2h_core

#endif

