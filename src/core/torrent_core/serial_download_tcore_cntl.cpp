#include "serial_download_tcore_cntl.hpp"
#include "torrent_core_config.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>

#include <boost/bind.hpp>

/**
 * Private hidden serial_download_tcore_cntl api 
 */

namespace t2h_core {

/**
 * Public serial_download_tcore_cntl api
 */

serial_download_tcore_cntl::serial_download_tcore_cntl(setting_manager_ptr setting_manager) : 
	base_torrent_core_cntl(), 
	setting_manager_(setting_manager), 
	session_ref_(NULL), 	
	extended_info_map_(),
	settings_() 
{
}
	
serial_download_tcore_cntl::~serial_download_tcore_cntl() 
{
}

void serial_download_tcore_cntl::set_core_session(libtorrent::session * session_ref) 
{
	update_settings();
	session_ref_ = session_ref;
}

int serial_download_tcore_cntl::availables_categories() const 
{
	using libtorrent::alert;
	/* Tell libtorrent session we are ready to dispatch all alers(eg notifications) */
	return (alert::all_categories & 
			~(alert::dht_notification + 
			alert::debug_notification +
			alert::stats_notification +
			alert::rss_notification));
} 

void serial_download_tcore_cntl::on_setup_core_session(libtorrent::session_settings & settings) 
{
	settings.prioritize_partial_pieces = true;
	settings.upload_rate_limit = 0;
}

void serial_download_tcore_cntl::dispatch_alert(libtorrent::alert * alert) 
{
	using namespace libtorrent;
	
	if (!alert) {
		TCORE_WARNING("passed not valid alert to dispatcher")
		return;
	}
	
	if (file_completed_alert * file_completed = alert_cast<file_completed_alert>(alert)) 
	{
		//on_file_complete(file_completed);
	}
	else if (add_torrent_alert * add_alert = alert_cast<add_torrent_alert>(alert)) 
	{
		on_add_torrent(add_alert);
	} 
	else if (torrent_finished_alert * tor_finised_alert = alert_cast<torrent_finished_alert>(alert)) 
	{
		on_finished(tor_finised_alert);
	}
	else if (piece_finished_alert * piece_fin_alert = alert_cast<piece_finished_alert>(alert)) 
	{
		on_piece_finished(piece_fin_alert);
	} 
	else if (state_changed_alert * state_ched_alert = alert_cast<state_changed_alert>(alert)) 
	{
		/** TODO investigate this case */
	}
	else if (torrent_deleted_alert * deleted_alert = alert_cast<torrent_deleted_alert>(alert)) 
	{
		on_deleted(deleted_alert);
	} 
	else if (state_update_alert * state_alert = alert_cast<state_update_alert>(alert)) 
	{
		on_update(state_alert);
	}
	else 
	{
		not_dispatched_alert_came(alert);
	}
}

void serial_download_tcore_cntl::update_settings() 
{
	settings_.max_async_download_size =
		setting_manager_->get_value<boost::int64_t>("tc_max_async_download_size");
}

/**
 * Private serial_download_tcore_cntl api
 */

void serial_download_tcore_cntl::on_recv(libtorrent::metadata_received_alert * alert) 
{
} 

void serial_download_tcore_cntl::on_add_torrent(libtorrent::add_torrent_alert * alert) 
{
	using libtorrent::torrent_handle;
	
	torrent_handle handle = alert->handle;
	
	/*  Set blob of pieces to max prior, others to low prior. 
		This if more better way as setup one by one piece to max prior  */
	if (alert->error || !handle.is_valid()) {
		TCORE_WARNING("adding torrent failed with error")
		return;
	}	
	
	details::extended_info_ptr extended_info(new (std::nothrow) details::extended_torrent_info());
	if (extended_info) { 
		extended_info->path = handle.save_path();
		if (!utility::container_safe_insert(extended_info_map_, extended_info->path, extended_info)) 
		{
			TCORE_WARNING("can not prioritize torrent (%s)," 
				"for this torrent the extended info entry already exist : [%s]", 
				handle.name().c_str(), handle.save_path().c_str())
			return;
		} // !if
		prioritize_files_at_start(handle);
		prioritize_pieces_at_start(handle, extended_info);
	}

	session_ref_->post_torrent_updates();
}

void serial_download_tcore_cntl::on_finished(libtorrent::torrent_finished_alert * alert) 
{
	TCORE_TRACE("finished '%s'", alert->handle.save_path().c_str())
	std::string const path = alert->handle.save_path();	
	utility::container_safe_remove(extended_info_map_, path);
} 

void serial_download_tcore_cntl::on_pause(libtorrent::torrent_paused_alert * alert) 
{
	TCORE_TRACE("alert pause")
}

void serial_download_tcore_cntl::on_update(libtorrent::state_update_alert * alert) 
{
	TCORE_TRACE("update came")
}

void serial_download_tcore_cntl::on_piece_finished(libtorrent::piece_finished_alert * alert) 
{
	if (!utility::container_safe_find(extended_info_map_, alert->handle.save_path(), 
			boost::bind(&serial_download_tcore_cntl::on_piece_finished_safe, 
				this, boost::ref(alert->handle), _1)))
	{
		/*  TODO Should never happen, but who knows mb need prioritize 
			all pieces to '1'(normal prior) in this case? */
		TCORE_WARNING("could not find extended info for '%s' torrent", 
			alert->handle.save_path().c_str())	
	} 	
}

void serial_download_tcore_cntl::on_deleted(libtorrent::torrent_deleted_alert * alert) 
{
	TCORE_TRACE("deleted '%s'", alert->handle.save_path().c_str())	
	std::string const path = alert->handle.save_path();
	utility::container_safe_remove(extended_info_map_, path);
}

void serial_download_tcore_cntl::not_dispatched_alert_came(libtorrent::alert * alert) 
{
}

void serial_download_tcore_cntl::on_piece_finished_safe(
	libtorrent::torrent_handle & handle, details::torrents_ex_iterator it) 
{
	details::extended_info_ptr ex_info = it->second; 
	if (ex_info->pieces_per_download == ex_info->downloaded_pieces) 
	{
		start_download_next_pieces(handle, ex_info); 
		ex_info->downloaded_pieces = 0;
	} 
	else // follow of the sequential download 
	{
		++ex_info->downloaded_pieces;	
	} 
	++ex_info->all_downloaded_pieces;	
	session_ref_->post_torrent_updates();
}

void serial_download_tcore_cntl::start_download_next_pieces(
	libtorrent::torrent_handle & handle, details::extended_info_ptr & extended_info) 
{
	int end = (extended_info->pieces_offset + extended_info->pieces_per_download) + 1;
	for (int it = extended_info->pieces_offset;
			it != end;
			++it)
	{
		handle.piece_priority(it, details::extended_torrent_info::max_prior);
	}
	extended_info->pieces_offset += extended_info->pieces_per_download;
}

void serial_download_tcore_cntl::prioritize_pieces_at_start(
		libtorrent::torrent_handle & handle, details::extended_info_ptr & extended_info) 
{
	using libtorrent::torrent_info;

	torrent_info const & info = handle.get_torrent_info();
	int const piece_size_offset = info.piece_length();
	int piece_size = piece_size_offset;
	extended_info->pieces = info.num_pieces(); 
	
	/** Find out which number of pieces we can download in async mode, 
		and prioritize pieces */
	for (;;) {
		if (piece_size >= settings_.max_async_download_size) 
			break;
		piece_size += piece_size_offset;
		++extended_info->pieces_per_download;
	}
	extended_info->pieces_offset = extended_info->pieces_per_download;
	
	for (int it = 0, end = extended_info->pieces; 
		it != end; 
		++it)
	{
		if (it <= extended_info->pieces_per_download) {
			handle.piece_priority(it, details::extended_torrent_info::max_prior);
			continue;
		}
		handle.piece_priority(it, details::extended_torrent_info::min_prior);
	}
}

void serial_download_tcore_cntl::prioritize_files_at_start(libtorrent::torrent_handle & handle) 
{
	libtorrent::torrent_info const & ti = handle.get_torrent_info();
	std::size_t const files_size = ti.num_files();
	for (std::size_t it = 0; it < files_size; ++it) 
		handle.file_priority(it, details::file_ex_info::min_prior);
}

void serial_download_tcore_cntl::restore_default_priority(libtorrent::torrent_handle & handle) 
{
	libtorrent::torrent_info const info = handle.get_torrent_info();
	std::size_t const pieces_size = (std::size_t)info.num_pieces() + 1;
	for (std::size_t it = 0; it < pieces_size; ++it)
		handle.piece_priority(it, details::extended_torrent_info::normal_prior);
}

}// namespace t2h_core 

