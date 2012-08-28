#ifndef SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED
#define SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED

#include "setting_manager.hpp"
#include "torrent_core_future.hpp"
#include "basic_safe_container.hpp"
#include "base_torrent_core_cntl.hpp"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

struct future_queue_traits {
	typedef boost::unordered_map<std::string, details::torrent_core_future_ptr> object_type;
	typedef details::torrent_core_future_ptr item_type;
	typedef item_type item_ptr;
	typedef item_type & item_ref;
	typedef item_type const & item_cref;
	typedef std::string find_type;
};

typedef future_queue_traits::object_type::iterator futures_iterator;
typedef future_queue_traits::object_type::const_iterator futures_const_iterator;
typedef utility::basic_safe_container<future_queue_traits> futures_queue;

struct static_settings {
	std::string tc_root;
	boost::int64_t max_async_download_size; 
};

} // namespace details

/**
 *
 */
class serial_download_tcore_cntl : public base_torrent_core_cntl {
public :
	serial_download_tcore_cntl();
	explicit serial_download_tcore_cntl(setting_manager_ptr setting_manager);
	virtual ~serial_download_tcore_cntl();
	
	virtual void set_core_session(libtorrent::session * session_ref);
	virtual void on_setup_core_session(libtorrent::session_settings & settings);

	virtual int availables_categories() const;

	virtual bool add_torrent(
		libtorrent::add_torrent_params const & params, libtorrent::torrent_handle & handle);

	virtual void dispatch_alert(libtorrent::alert * alert);

	void update_settings();
	
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

	/** */	
	bool setup_torrent(libtorrent::torrent_handle & handle);

	setting_manager_ptr setting_manager_;
	libtorrent::session * session_ref_;
	details::futures_queue futures_;
	details::static_settings mutable settings_;

};

} // namespace t2h_core

#endif

