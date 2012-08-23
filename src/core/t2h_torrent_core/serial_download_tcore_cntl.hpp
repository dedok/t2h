#ifndef SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED
#define SERIAL_DOWNLOAD_TCORE_CNTL_HPP_INCLUDED

#include "base_torrent_core_cntl.hpp"
#include "t2h_settings_manager.hpp"
#include "basic_safe_container.hpp"

#include <map>

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

enum torrent_status {
	in_process = 0x1,
	status_stop,
	status_pause,
	status_remove,
	status_resume,
	status_unknown = status_resume + 1
};

struct extended_torrent_info 
	: public boost::enable_shared_from_this<extended_torrent_info>
{
	static int const max_prior = 5;
	static int const normal_prior = 1;
	static int const min_prior = 1;

	extended_torrent_info() 
		: pieces(0), 
		pieces_offset(0), 
		downloaded_pieces(0), 
		pieces_per_download(1),
		all_downloaded_pieces(0),
		status(status_unknown),
		path()
	{
	}

	int pieces;	
	int pieces_offset;
	int downloaded_pieces;
	int pieces_per_download;
	int all_downloaded_pieces;

	torrent_status status;
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

struct torrents_index_list_traits {
	typedef std::map<std::size_t, std::string> object_type;
	typedef std::string item_type;
	typedef item_type * item_ptr;
	typedef item_type & item_ref;
	typedef item_type const & item_cref;
	typedef std::size_t find_type;
};

typedef utility::basic_safe_container<torrents_map_ex_traits> torrents_map_ex;
typedef utility::basic_safe_container<torrents_index_list_traits> torrents_index_list;

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
	
	virtual void post_pause_download(std::string const & torrent_name);
	virtual void post_resume_download(std::string const & torrent_name);
	virtual void post_stop_download(std::string const & torrent_name);
	virtual void post_remove_torrent(std::string const & torrent_name);

	virtual std::size_t decode_id(std::string const & torrent_name);
	virtual std::string encode_id(std::size_t torrent_id);

	void update_settings();

private :
	/** Functions for dispatching notification from core_session */
	void on_recv(libtorrent::metadata_received_alert * alert); 
	void on_add_torrent(libtorrent::add_torrent_alert * alert);
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
	void restore_default_priority(libtorrent::torrent_handle & handle);

	/** */
	void post_new_status(std::string const & torrent_name, details::torrent_status new_status);
	void post_new_status_unsafe(std::string const & torrent_name, details::torrent_status new_status);
	
	setting_manager_ptr setting_manager_;
	libtorrent::session * session_ref_;
	details::torrents_map_ex extended_info_map_;
	details::torrents_index_list indexes_list_;	
	details::static_settings mutable settings_;
};

} // namespace t2h_core

#endif

