#include "serial_download_tcore_cntl.hpp"
#include "torrent_core_config.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>


namespace t2h_core {

/**
 * Private hidden serial_download_tcore_cntl api 
 */
namespace details {

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
 * Public serial_download_tcore_cntl api
 */

serial_download_tcore_cntl::serial_download_tcore_cntl(setting_manager_ptr setting_manager) : 
	base_torrent_core_cntl(), 
	setting_manager_(setting_manager), 
	session_ref_(NULL), 	
	futures_(),
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
}

bool serial_download_tcore_cntl::add_torrent(
	libtorrent::add_torrent_params const & params, libtorrent::torrent_handle & handle) 
{
	details::torrent_core_future_ptr future(new details::add_torrent_future());
	if (!utility::container_safe_insert(futures_, params.save_path, future)) { 
		TCORE_WARNING("serial_download_tcore_cntl insert to future list failed")
		return false;
	}
	
	session_ref_->async_add_torrent(params);
	future->wait_result();
	details::add_torrent_future_ptr add_future = 
		boost::static_pointer_cast<details::add_torrent_future>(future);
	handle = add_future->handle;

	utility::container_safe_remove(futures_, params.save_path);	
	return add_future->state;
}

void serial_download_tcore_cntl::dispatch_alert(libtorrent::alert * alert) 
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
	else
		not_dispatched_alert_came(alert);
	
	session_ref_->post_torrent_updates();
}

bool serial_download_tcore_cntl::setup_torrent(libtorrent::torrent_handle & handle) 
{
	std::vector<int> file_priorities = handle.file_priorities();
	std::fill(file_priorities.begin(), file_priorities.end(), 0);
	handle.prioritize_files(file_priorities);

	handle.set_max_connections(50);
	handle.set_max_uploads(-1);
	handle.set_upload_limit(0);
	handle.set_download_limit(65565);
	handle.set_sequential_download(true);

	return true;
}

void serial_download_tcore_cntl::update_settings() 
{
	settings_.max_async_download_size =
		setting_manager_->get_value<boost::int64_t>("tc_max_async_download_size");
	settings_.tc_root = setting_manager_->get_value<std::string>("tc_root");
}

/**
 * Private serial_download_tcore_cntl api
 */

void serial_download_tcore_cntl::on_metadata_recv(libtorrent::metadata_received_alert * alert) 
{	
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
	details::save_file(torrent_file_path, buffer);
} 

void serial_download_tcore_cntl::on_add_torrent(libtorrent::add_torrent_alert * alert) 
{
	using libtorrent::torrent_handle;

	torrent_handle handle = alert->handle;		
	details::add_torrent_future_ptr add_future;
	
	if (alert->error || !handle.is_valid()) 
		return;

	{ // futures_ lock zone
	boost::lock_guard<boost::mutex> guard(futures_.lock);
	details::futures_iterator found = utility::container_find_unsafe(futures_, handle.save_path());
	if (found == futures_.cont.end()) {
		TCORE_TRACE("failed to add torrent by path '%s', can not find future routine", 
			alert->handle.save_path().c_str())
		return;
	} 
	add_future = details::future_cast<details::add_torrent_future>(found->second);
	} // futures_ lock zone end
	
	details::scoped_future_release sfr(add_future);	
	
	if ((add_future->state = setup_torrent(handle))) 
		add_future->handle = handle;
}

void serial_download_tcore_cntl::on_finished(libtorrent::torrent_finished_alert * alert) 
{
	TCORE_TRACE("torrent finished '%s'", alert->handle.save_path().c_str())
	std::string const path = alert->handle.save_path();	
} 

void serial_download_tcore_cntl::on_pause(libtorrent::torrent_paused_alert * alert) 
{
	TCORE_TRACE("alert pause")
}

void serial_download_tcore_cntl::on_update(libtorrent::state_update_alert * alert) 
{
	using libtorrent::torrent_status;

	typedef std::vector<torrent_status> statuses_type;

	statuses_type statuses = alert->status;
	for (statuses_type::iterator it = statuses.begin(), last = statuses.end();
		it != last;
		++it) 
	{
		std::string const paused( it->paused ? "yes" : "no");
		std::cout << "State : " << (int)it->state << " paused : " << paused << std::endl;
		std::cout << "Download rate " << it->download_rate << std::endl;
	}
}

void serial_download_tcore_cntl::on_piece_finished(libtorrent::piece_finished_alert * alert) 
{
	std::cout << "Path " << alert->handle.save_path() 
		<< " Index downloaded : " << alert->piece_index << std::endl;
}

void serial_download_tcore_cntl::on_file_complete(libtorrent::file_completed_alert * alert)
{

}

void serial_download_tcore_cntl::on_deleted(libtorrent::torrent_deleted_alert * alert) 
{
	std::string const path = alert->handle.save_path();
}

void serial_download_tcore_cntl::on_state_change(libtorrent::state_changed_alert * alert) 
{
	// TODO imvestigate this case
}	

void serial_download_tcore_cntl::not_dispatched_alert_came(libtorrent::alert * alert) 
{
//	TCORE_TRACE("event : '%i'", (int)alert->type())
}

}// namespace t2h_core 

