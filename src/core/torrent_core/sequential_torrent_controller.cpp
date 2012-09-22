#include "sequential_torrent_controller.hpp"

#include "lookup_error.hpp"
#include "misc_utility.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>

namespace t2h_core {

/**
 * Private hidden sequential_torrent_controller api 
 */
namespace details {

static inline void log_state_update_alerts(libtorrent::torrent_status const & status) 
{
	using libtorrent::torrent_status;

	TCORE_TRACE("Updated state for torrent '%s' :"
		"Paused '%s'\nTorrent state '%i'\nSequential download '%i'\nProgress '%f'\n"
		"Download rate '%i'\nNum completed '%i'",
		status.handle.save_path().c_str(),
		status.paused ? "true" : "false",
		(int)status.state,
		status.sequential_download,
		status.progress,
		status.download_rate,
		status.num_complete)
}

static inline int save_file(std::string const & filename, std::vector<char> & bytes)
{
	libtorrent::file file;
	boost::system::error_code error_code;
	if (!file.open(filename, libtorrent::file::write_only, error_code)) { 
		if (error_code) 
			return -1;
		libtorrent::file::iovec_t filb = { &bytes[0], bytes.size() };
		libtorrent::size_type const written = file.writev(0, &filb, 1, error_code);
		return (error_code ? -3 : written);
	}
	return -1;
}

} // namespace details

/**
 * Public sequential_torrent_controller api
 */

sequential_torrent_controller::sequential_torrent_controller(setting_manager_ptr setting_manager) : 
	base_torrent_core_cntl(), 
	setting_manager_(setting_manager), 
	session_ref_(NULL), 	
	shared_buffer_ref_(NULL),
	settings_()
{
}
	
sequential_torrent_controller::~sequential_torrent_controller() 
{
	session_ref_ = NULL; 
	shared_buffer_ref_ = NULL;
}

int sequential_torrent_controller::availables_categories() const 
{
	using libtorrent::alert;
	/* Tell libtorrent session we are ready to dispatch follow categories */
	return (alert::all_categories & 
			~(alert::dht_notification +
#if defined(T2H_CORE_NO_DETAILED_PROGRESS_NOTIFICATIONS)
			alert::progress_notification +
#endif
#if defined(T2H_DEBUG) && defined(T2H_DEEP_DEBUG)
			alert::debug_notification +
#endif
			alert::stats_notification));
} 

void sequential_torrent_controller::set_session(libtorrent::session * session_ref) 
{
	update_settings();
	session_ref_ = session_ref;
}

void sequential_torrent_controller::set_shared_buffer(details::shared_buffer * shared_buffer_ref) 
{
	shared_buffer_ref_ = shared_buffer_ref;
}

void sequential_torrent_controller::on_setup_core_session(libtorrent::session_settings & settings) 
{
	if (settings_.sequential_download)
		settings.prioritize_partial_pieces = true;
}

bool sequential_torrent_controller::add_torrent(details::torrent_ex_info_ptr ex_info)
{		
	using details::future_cast;
	using details::add_torrent_future;
	using boost::posix_time::seconds;
	
	// TODO add coments 
	details::scoped_future_promise_init<details::add_torrent_future> scoped_promise(ex_info->future); 
	session_ref_->async_add_torrent(ex_info->torrent_params);
	boost::system_time const timeout = 
		boost::get_system_time() + seconds(settings_.futures_timeouts.torrent_add_timeout);
	if (ex_info->future->timed_wait_result(timeout)) {
		ex_info->handle = future_cast<add_torrent_future>(ex_info->future)->handle;
		return ex_info->handle.is_valid();
	}

	return false;
}

void sequential_torrent_controller::dispatch_alert(libtorrent::alert * alert) 
{
	using namespace libtorrent;
	
	if (torrent_paused_alert * paused = alert_cast<torrent_paused_alert>(alert))  
		on_pause(paused);
	else if (metadata_received_alert * mra = alert_cast<metadata_received_alert>(alert))
		on_metadata_recv(mra);
	else if (file_completed_alert * file_completed = alert_cast<file_completed_alert>(alert)) 
		on_file_complete(file_completed);
	else if (add_torrent_alert * add_alert = alert_cast<add_torrent_alert>(alert)) 
		on_add_torrent(add_alert);
	else if (torrent_finished_alert * tor_finised_alert = alert_cast<torrent_finished_alert>(alert)) 
		on_finished(tor_finised_alert);
	else if (piece_finished_alert * piece_fin_alert = alert_cast<piece_finished_alert>(alert)) 
		on_piece_finished(piece_fin_alert);
	else if (state_changed_alert * state_ched_alert = alert_cast<state_changed_alert>(alert))  
		on_state_change(state_ched_alert);
	else if (torrent_deleted_alert * deleted_alert = alert_cast<torrent_deleted_alert>(alert)) 
		on_deleted(deleted_alert);
	else if (state_update_alert * state_alert = alert_cast<state_update_alert>(alert)) 
		on_update(state_alert);
}

void sequential_torrent_controller::setup_torrent(libtorrent::torrent_handle & handle) 
{
	if (settings_.partial_files_download) {
		std::vector<int> file_priorities = handle.file_priorities();
		std::fill(file_priorities.begin(), file_priorities.end(), 0);
		handle.prioritize_files(file_priorities);
	}

	handle.set_max_connections(settings_.max_connections_per_torrent);
	handle.set_max_uploads(settings_.max_uploads); 
	handle.set_upload_limit(settings_.upload_limit); 
	handle.set_download_limit(settings_.download_limit); 
	handle.set_sequential_download(settings_.sequential_download); 
#if  !defined(TORRENT_DISABLE_RESOLVE_COUNTRIES)
	handle.resolve_countries(settings_.resolve_countries); 
#endif
}

void sequential_torrent_controller::update_settings() 
{
	// TODO add more settings
	settings_.max_async_download_size =
		setting_manager_->get_value<boost::int64_t>("tc_max_async_download_size");
	settings_.tc_root = setting_manager_->get_value<std::string>("tc_root");
	settings_.auto_error_resolving = true;
	settings_.resolve_checkout = 20;
	settings_.resolve_countries = true;
	settings_.sequential_download = true;
	settings_.download_limit = 0;
	settings_.upload_limit = 0;
	settings_.max_uploads = -1;
	settings_.max_connections_per_torrent = 50;
	settings_.partial_files_download = false;
	settings_.futures_timeouts.torrent_add_timeout = 3;
}

/**
 * Private sequential_torrent_controller api
 */

void sequential_torrent_controller::on_metadata_recv(libtorrent::metadata_received_alert * alert) 
{	
	TCORE_TRACE("Torrent '%s' recv. metadata", alert->handle.get_torrent_info().name().c_str())

	std::vector<char> buffer;
	libtorrent::torrent_handle & handle = alert->handle;
	libtorrent::torrent_info const & torrent_info = handle.get_torrent_info();
	libtorrent::create_torrent torrent(torrent_info);
	libtorrent::entry torrent_enrty = torrent.generate();
		
	libtorrent::bencode(std::back_inserter(buffer), torrent_enrty);
	std::string torrent_file_path = torrent_info.name() + "." + 
		libtorrent::to_hex(torrent_info.info_hash().to_string()) + 
		".torrent";

	torrent_file_path = libtorrent::combine_path(settings_.tc_root, torrent_file_path);
	if (details::save_file(torrent_file_path, buffer) == -1) {
		TCORE_WARNING("can not save torrent meta to file, torrent name '%s'",
			alert->handle.get_torrent_info().name().c_str())
		return;
	}		
} 

void sequential_torrent_controller::on_add_torrent(libtorrent::add_torrent_alert * alert) 
{
	TCORE_TRACE("Torrent '%s' adding", alert->handle.save_path().c_str())

	using details::future_cast;
	using libtorrent::torrent_handle;	
	using details::add_torrent_future_ptr;
	using details::scoped_future_promise_init;
	
	torrent_handle handle = alert->handle;		
	details::add_torrent_future_ptr add_future;
	
	if (alert->error || !handle.is_valid()) { 
		TCORE_WARNING("Torrent '%s' add failed with error '%i'", alert->handle.save_path().c_str(), alert->error.value())
		return;
	}

	// TODO may be need to re-add torrent in follow two cases?	
	details::torrent_ex_info_ptr ex_info = shared_buffer_ref_->get(handle.save_path());
	if (!ex_info) {
		TCORE_WARNING("Torrent '%s' add failed, can not find extended info", handle.save_path().c_str())
		session_ref_->remove_torrent(handle);
		return;
	}

	if (!ex_info->future) {
		TCORE_WARNING("Torrent '%s' have not future promise", handle.save_path().c_str())
		shared_buffer_ref_->remove(handle.save_path());	
		session_ref_->remove_torrent(handle);
		return;
	} 
	
	details::scoped_future_release future_done_release(ex_info->future);	
	details::add_torrent_future_ptr atf_ptr = details::future_cast<details::add_torrent_future>(ex_info->future);
	setup_torrent(handle);
	atf_ptr->handle = handle;
}

void sequential_torrent_controller::on_finished(libtorrent::torrent_finished_alert * alert) 
{
	TCORE_TRACE("Torrent finished '%s'", alert->handle.save_path().c_str())
	std::string const path = alert->handle.save_path();	
} 

void sequential_torrent_controller::on_pause(libtorrent::torrent_paused_alert * alert) 
{
	TCORE_TRACE("Torrent '%s' paused", alert->handle.save_path().c_str())
}

void sequential_torrent_controller::on_update(libtorrent::state_update_alert * alert) 
{
	using namespace libtorrent;
	
	typedef std::vector<torrent_status> statuses_type;
	
	details::torrent_ex_info_ptr ex_info;
	statuses_type statuses = alert->status;

	for (statuses_type::iterator it = statuses.begin(), last = statuses.end();
		it != last;
		++it) 
	{
#if defined(T2H_DEBUG) && defined(T2H_DEEP_DEBUG)
		details::log_state_update_alerts(*it);
#endif
		if (!(ex_info = shared_buffer_ref_->get(it->handle.save_path()))) {
			TCORE_WARNING("Can not find extended info for torrent '%s'", it->handle.save_path().c_str())
			continue;
		}
		// TODO improve auto resolving logic
		if (settings_.auto_error_resolving && 
			(utility::get_current_time() >= ex_info->last_resolve_checkout || ex_info->resolver)) 
		{
			if (!ex_info->resolver)
				ex_info->last_resolve_checkout = 
					utility::get_current_time() + boost::posix_time::seconds(settings_.resolve_checkout);
			details::lookup_error lookuper(ex_info, *it);
		}

		on_torrent_status_changes(ex_info);	
	} // for
}

void sequential_torrent_controller::on_torrent_status_failure(details::torrent_ex_info_ptr ex_info) 
{
	// TODO add failure callbacks & actions	
}

void sequential_torrent_controller::on_torrent_status_changes(details::torrent_ex_info_ptr ex_info) 
{
	// TODO add progress callbacks
}

void sequential_torrent_controller::on_piece_finished(libtorrent::piece_finished_alert * alert) 
{
	TCORE_TRACE("Torrent '%s' finished download piece '%i'", 
		alert->handle.save_path().c_str(), alert->piece_index)
}

void sequential_torrent_controller::on_file_complete(libtorrent::file_completed_alert * alert)
{
	TCORE_TRACE("Torrent '%s' complete download file '%i'", alert->handle.save_path().c_str(), alert->index)
}

void sequential_torrent_controller::on_deleted(libtorrent::torrent_deleted_alert * alert) 
{
	TCORE_TRACE("Torrent '%s' deleted", alert->handle.save_path().c_str())
	std::string const path = alert->handle.save_path();
	shared_buffer_ref_->remove(path);
}

void sequential_torrent_controller::on_state_change(libtorrent::state_changed_alert * alert) 
{
	// TODO investigate this case
}	

}// namespace t2h_core 

