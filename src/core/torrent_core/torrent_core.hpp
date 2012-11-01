#ifndef TORRENT_CORE_HPP_INCLUDED
#define TORRENT_CORE_HPP_INCLUDED

#include "base_service.hpp"
#include "shared_buffer.hpp"
#include "setting_manager.hpp"
#include "torrent_core_config.hpp"
#include "base_torrent_core_cntl.hpp"
#include "torrent_core_event_handler.hpp"

#if defined(__GNUG__)
#	pragma GCC system_header
#endif

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include <boost/thread.hpp>
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
	bool loadable_session;
};

} // namespace details

/** The main torrent_core parameters, all filds of the t2h_core::torrent_core_params 
	must set to valid values, otherwise SEGFAULT ... */ 
struct torrent_core_params {
	setting_manager_ptr setting_manager;
	base_torrent_core_cntl_ptr controller;
	torrent_core_event_handler_ptr event_handler;
};

/**
 * 
 */
class torrent_core : public common::base_service {
public :
	enum { invalid_torrent_id = 0x1001 };
	static const char * this_service_name;
	typedef std::size_t size_type;

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
	
	size_type add_torrent(boost::filesystem::path const & path);
	size_type add_torrent_url(std::string const & url);
	
	std::string get_torrent_info(size_type torrent_id) const;

	std::string start_torrent_download(size_type torrent_id, int file_id);
	void pause_download(size_type torrent_id, int file_id);
	void resume_download(size_type torrent_id, int file_id);	
	void remove_torrent(size_type torrent_id);
	void stop_torrent_download(size_type torrent_id);
	
private :
	bool init_core_session();
	void setup_core_session();
	bool init_torrent_core_settings();
	void s11z_session_state();

	void core_main_loop();
	void handle_core_notifications();	
	bool is_critical_error(libtorrent::alert * alert);
	void handle_critical_error_notification(libtorrent::alert * alert);

	torrent_core_params mutable params_;			
	base_service::service_state volatile mutable cur_state_;
	details::torrent_core_settings settings_;		
	boost::mutex mutable core_lock_;
	details::shared_buffer * shared_buffer_;
	libtorrent::session * core_session_;
	boost::condition_variable core_session_loop_wait_;
	boost::scoped_ptr<boost::thread> core_session_loop_;

};

typedef boost::shared_ptr<torrent_core> torrent_core_ptr;

} // namespace t2h_core

#endif

