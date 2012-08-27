#ifndef SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED
#define SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED

#include "setting_manager.hpp"
#include "basic_safe_container.hpp"
#include "base_torrent_core_cntl.hpp"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

struct extended_torrent_info 
	: public boost::enable_shared_from_this<extended_torrent_info>
{
	static int const max_prior = 5;
	static int const normal_prior = 1;
	static int const min_prior = 1;
	
	extended_torrent_info() : 
		pieces(0), 
		pieces_offset(0), 
		downloaded_pieces(0), 
		pieces_per_download(1),
		all_downloaded_pieces(0),
		path()
	{ }

	int pieces;	
	int pieces_offset;
	int downloaded_pieces;
	int pieces_per_download;
	int all_downloaded_pieces;
	std::string path;

};

typedef boost::shared_ptr<extended_torrent_info> extended_info_ptr;

struct torrents_map_ex_traits {
	typedef boost::unordered_map<std::string, extended_info_ptr> object_type;
	typedef extended_info_ptr item_type;
	typedef item_type item_ptr;
	typedef item_type & item_ref;
	typedef item_type const & item_cref;
	typedef std::string find_type;
};

typedef torrents_map_ex_traits::object_type::iterator torrents_ex_iterator;
typedef torrents_map_ex_traits::object_type::const_iterator torrents_ex_citerator;
typedef utility::basic_safe_container<torrents_map_ex_traits> torrents_map_ex;

struct static_settings {
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
	virtual void dispatch_alert(libtorrent::alert * alert);
	
	virtual void on_add_torrent(libtorrent::add_torrent_alert * alert);
	void update_settings();

	
private :
	
	/** Functions for dispatching notification from core_session */
	void on_recv(libtorrent::metadata_received_alert * alert); 
	void on_finished(libtorrent::torrent_finished_alert * alert); 
	void on_pause(libtorrent::torrent_paused_alert * alert);
	void on_update(libtorrent::state_update_alert * alert);
	void on_piece_finished(libtorrent::piece_finished_alert * alert);
	void on_deleted(libtorrent::torrent_deleted_alert * alert);
	void not_dispatched_alert_came(libtorrent::alert * alert);

	void dispatch_torrent_ex_status(libtorrent::torrent_handle & handle, details::torrents_ex_iterator it);
	void on_piece_finished_safe(libtorrent::torrent_handle & handle, details::torrents_ex_iterator it);
	
	/** */
	void start_download_next_pieces(libtorrent::torrent_handle & handle, details::extended_info_ptr & extended_info); 
	void prioritize_pieces_at_start(libtorrent::torrent_handle & handle, details::extended_info_ptr & extended_info);
	void prioritize_files_at_start(libtorrent::torrent_handle & handle); 
	void restore_default_priority(libtorrent::torrent_handle & handle);

	setting_manager_ptr setting_manager_;
	libtorrent::session * session_ref_;
	details::torrents_map_ex extended_info_map_;
	details::static_settings mutable settings_;

};

} // namespace t2h_core

#endif

