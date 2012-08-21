#include "serial_download_tcore_cntl.hpp"
#include "t2h_torrent_core_config.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>

/**
 * Private hidden serial_download_tcore_cntl api 
 */

namespace t2h_core {

/**
 * Public serial_download_tcore_cntl api
 */

serial_download_tcore_cntl::serial_download_tcore_cntl(setting_manager_ptr setting_manager) 
	: base_torrent_core_cntl(), setting_manager_(setting_manager), settings_(), session_ref_(NULL), extended_info_map_()
{
}
	
serial_download_tcore_cntl::~serial_download_tcore_cntl() 
{
}

void serial_download_tcore_cntl::set_core_session(libtorrent::session * session_ref) 
{
	// setup session reference and update controller settings
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
		
	if (add_torrent_alert * add_alert = 
		alert_cast<add_torrent_alert>(alert)) 
	{
		on_add(add_alert);
	} 
	else if (torrent_finished_alert * tor_finised_alert 
		= alert_cast<torrent_finished_alert>(alert)) 
	{
		on_finished(tor_finised_alert);
	}
	else if (piece_finished_alert * piece_fin_alert = 
		alert_cast<piece_finished_alert>(alert)) 
	{
		on_piece_finished(piece_fin_alert);
	} 
	else if (state_changed_alert * state_ched_alert 
		= alert_cast<state_changed_alert>(alert)) 
	{
		on_state_changed_alert(state_ched_alert);
	}
	else not_dispatched_alert_came(alert);
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

void serial_download_tcore_cntl::on_add(libtorrent::add_torrent_alert * alert) 
{
	using libtorrent::torrent_handle;

	/*  Set blob of pieces to max prior, other low prior. 
		This if more better way as setup one by one piece to max prior  */

	if (alert->error) {
		TCORE_WARNING("adding torrent failed with error")
		return;
	}
	
	torrent_handle handle = alert->handle;
	if (!handle.is_valid()) {
		TCORE_WARNING("add & setup torrent failed handle not valid")
		return;
	}
	
	details::extended_info_ptr extended_info( 
		new (std::nothrow) details::extended_torrent_info());
	if (extended_info) 
		prioritize_pieces_at_start(handle, extended_info);
	session_ref_->post_torrent_updates();
}

void serial_download_tcore_cntl::on_finished(libtorrent::torrent_finished_alert * alert) 
{
	TCORE_TRACE("finished")
} 

void serial_download_tcore_cntl::on_pause(libtorrent::torrent_paused_alert * alert) 
{
	TCORE_TRACE("alert")
}

void serial_download_tcore_cntl::on_update(libtorrent::state_update_alert * alert) 
{
}

void serial_download_tcore_cntl::on_piece_finished(libtorrent::piece_finished_alert * alert) 
{
	using libtorrent::torrent_handle;

	torrent_handle handle;
	extended_info_map_type::iterator found;

	if (!(handle = alert->handle).is_valid()) {
		TCORE_WARNING("could not setup next pieces, handle is not valid")
		return;
	} 
	
	found = extended_info_map_.find(handle.save_path());
	if (found != extended_info_map_.end()) 
	{
		if (found->second->pieces_per_download == found->second->downloaded_pieces) 
		{ 
			start_download_next_pieces(handle, found->second); 
			found->second->downloaded_pieces = 0;
			session_ref_->post_torrent_updates();
		} 
		else // follow of the sequential download 
		{
			++found->second->downloaded_pieces;
		} 
		++found->second->all_downloaded_pieces;	
	}
	else // panic case follow
	{
		/*  TODO Should never happen, but who knows mb need prioritize 
			all pieces to '1'(normal prior) in this case? */
		TCORE_WARNING("could not find extended info for '%s' torrent", 
			handle.save_path().c_str())	
		session_ref_->post_torrent_updates();
	} 	
}

void serial_download_tcore_cntl::not_dispatched_alert_came(libtorrent::alert * alert) 
{
}

void serial_download_tcore_cntl::on_state_changed_alert(libtorrent::alert * alert) 
{
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
	std::string const save_path = handle.save_path();
		
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
	if (extended_info_map_.find(save_path) == extended_info_map_.end()) 
	{
		extended_info_map_[save_path] = extended_info;
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
	else // !if
	{
		TCORE_WARNING("could not prioritize torrent(%s)," 
			"for this torrent the extended info entry already exist : [%s]", 
			handle.name().c_str(), handle.save_path().c_str())
	}
}


} // namespace t2h_core 

