#ifndef SEQUENTIAL_TORRENT_CONTROLLER_HPP_INCLUDED
#define SEQUENTIAL_TORRENT_CONTROLLER_HPP_INCLUDED

#include "setting_manager.hpp"
#include "base_torrent_core_cntl.hpp"
#include "torrent_core_macros.hpp"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

/**
 * Controller hidden settings
 */
struct static_settings {
	std::string tc_root;							// Working root, must be a full path
	bool auto_error_resolving;						// Auto error detencting and resolving
	std::size_t resolve_checkout;					// Resolving check out duration for each torrent in queue, in seconds
	boost::int64_t max_async_download_size; 		// Not used for now, in kilobytes
	bool partial_files_download;					// Allow to partrial files download
	bool sequential_download;						// Allow sequential download
	bool resolve_countries;							// Allow to use resolve countries
	int download_limit;								// Download rate limit 0 = unlimit, in kb
	int upload_limit;								// Unpload rate limit 0 = unlimit, in kb
	int max_uploads;								// Max upload limit
	int max_connections_per_torrent;				// Max allow connection per torrent
	struct {
		std::size_t torrent_add_timeout;		 	// Torrent add promise timeout, in seconds
	} futures_timeouts;
};

} // namespace details

/**
 * sequential download controller 
 */
class sequential_torrent_controller : public base_torrent_core_cntl {
public :
	sequential_torrent_controller();
	explicit sequential_torrent_controller(setting_manager_ptr setting_manager);
	virtual ~sequential_torrent_controller();
	
	virtual int availables_categories() const;
	
	virtual bool set_session(libtorrent::session * session_ref);
	virtual void on_setup_core_session(libtorrent::session_settings & settings);

	virtual void set_shared_buffer(details::shared_buffer * buffer_ref);

	virtual bool add_torrent(details::torrent_ex_info_ptr ex_info);

	virtual void dispatch_alert(libtorrent::alert * alert);

private :
	
	/** Functions for dispatching notification from core_session */	
	void on_add_torrent(libtorrent::add_torrent_alert * alert);
	void on_metadata_recv(libtorrent::metadata_received_alert * alert); 
	void on_finished(libtorrent::torrent_finished_alert * alert); 
	void on_pause(libtorrent::torrent_paused_alert * alert);
	void on_update(libtorrent::state_update_alert * alert);
	void on_piece_finished(libtorrent::piece_finished_alert * alert);	
	void on_file_complete(libtorrent::file_completed_alert * alert);
	void on_deleted(libtorrent::torrent_deleted_alert * alert);
	void on_state_change(libtorrent::state_changed_alert * alert);	
	void not_dispatched_alert_came(libtorrent::alert * alert);
	void on_torrent_status_changes(details::torrent_ex_info_ptr ex_info);
	void on_torrent_status_failure(details::torrent_ex_info_ptr ex_info);

	/** Others funtions */	
	void setup_torrent(libtorrent::torrent_handle & handle);
	void update_settings();
	
	setting_manager_ptr setting_manager_;
	libtorrent::session * session_ref_;
	details::shared_buffer * shared_buffer_ref_;
	details::static_settings mutable settings_;
};

} // namespace t2h_core

#endif

