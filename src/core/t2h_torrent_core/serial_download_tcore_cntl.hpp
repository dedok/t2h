#ifndef SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED
#define SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED

#include "base_torrent_core_cntl.hpp"
#include "t2h_settings_manager.hpp"

#include <boost/cstdint.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

struct extended_torrent_info 
	: public boost::enable_shared_from_this<extended_torrent_info>
{
	extended_torrent_info() 
		: pieces(0), 
		pieces_offset(0), 
		downloaded_pieces(0), 
		pieces_per_download(1),
		all_downloaded_pieces(0)
	{
	}

	static int const max_prior = 5;
	static int const min_prior = 1;

	int pieces;	
	int pieces_offset;
	int downloaded_pieces;
	int pieces_per_download;
	int all_downloaded_pieces;
};

typedef boost::shared_ptr<details::extended_torrent_info> extended_info_ptr;

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
	
	void update_settings();

private :
	typedef boost::unordered_map<std::string, details::extended_info_ptr> extended_info_map_type;
	
	/** Functions for dispatching notification from core_session */
	void on_recv(libtorrent::metadata_received_alert * alert); 
	void on_add(libtorrent::add_torrent_alert * alert);
	void on_finished(libtorrent::torrent_finished_alert * alert); 
	void on_pause(libtorrent::torrent_paused_alert * alert);
	void on_update(libtorrent::state_update_alert * alert);
	void on_piece_finished(libtorrent::piece_finished_alert * alert);
	void not_dispatched_alert_came(libtorrent::alert * alert);
	void on_state_changed_alert(libtorrent::alert * alert);
	
	/** */
	void start_download_next_pieces(libtorrent::torrent_handle & handle, details::extended_info_ptr & extended_info); 
	void prioritize_pieces_at_start(libtorrent::torrent_handle & handle, details::extended_info_ptr & extended_info);

	setting_manager_ptr setting_manager_;
	libtorrent::session * session_ref_;
	extended_info_map_type extended_info_map_;
	details::static_settings mutable settings_;

};

} // namespace t2h_core

#endif

