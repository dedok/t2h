#include "torrent_core.hpp"
#include "misc_utility.hpp"
#include "torrent_core_macros.hpp"

namespace t2h_core {

namespace details {

static inline std::string create_random_path(std::string const & root_path) 
{ 
	boost::filesystem::path save_path = root_path; 
	return (save_path / utility::get_random_string()).string(); 
}
	
} // namespace details

/**
 * Public torrent_core api
 */

char const * torrent_core::this_service_name = "torrent_core";

torrent_core::torrent_core(torrent_core_params const & params) 
		: common::base_service(this_service_name), 
		params_(params),
		cur_state_(base_service::service_state_unknown),
		settings_(),
		torrents_map_(),
		thread_loop_(),
		core_session_(libtorrent::fingerprint("LT", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0), 
					libtorrent::session::add_default_plugins, 
					params.controller->availables_categories()
					) 
{
	params.controller->set_core_session(&core_session_);
}

torrent_core::~torrent_core() 
{ 
}

bool torrent_core::launch_service() 
{
	try 
	{
		if (thread_loop_) {
			TCORE_WARNING("fail to launch torrent core, torrent core already launced")
			return false;
		}
		
		if (init_core_session()) { 
			thread_loop_.reset(new boost::thread(&torrent_core::core_main_loop, this));
			cur_state_ = base_service::service_running;
		}
	}
	catch (libtorrent::libtorrent_exception const & expt) 
	{
		TCORE_ERROR("launching of the torrent core failed, with reason '%s'", expt.what())
		return false;
	}

	return (cur_state_ == base_service::service_running);
}

void torrent_core::stop_service() 
{
	/** Stop core_session_ subsytems, then core_session_ main loop. 
		NOTE: To stop all 'trackers' session need to call dtor of core_session_ */
	if (cur_state_ == base_service::service_running) {
#ifndef TORRENT_DISABLE_DHT
		core_session_.stop_dht(); 
#endif
		core_session_.stop_lsd();
		core_session_.stop_upnp(); core_session_.stop_natpmp();
		core_session_.post_torrent_updates();
		cur_state_ = base_service::service_stoped;
		wait_service();
	}
}

void torrent_core::wait_service() 
{
	if (!thread_loop_) return;
	thread_loop_->join();
}

torrent_core::ptr_type torrent_core::clone() 
{ 
	return ptr_type(new torrent_core(params_)); 
}

common::base_service::service_state torrent_core::get_service_state() const 
{
	return cur_state_;
}

int torrent_core::add_torrent(boost::filesystem::path const & path) 
{
	/** Setup torrent and envt. then async add new torrent by file path. 
		NOTE if torrent already in queue, the torrent will be add by force */
	using libtorrent::torrent_info;
	
	int torrent_id = details::invalid_torrent_id;	
	details::torrent_ex_info_ptr ex_info(new details::torrent_ex_info());
	
	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	if (cur_state_ != base_service::service_running) { 
		TCORE_WARNING("add torrent by path '%s' failed torrent core not runing", 
			path.string().c_str())
		return invalid_torrent_id;
	}
			
	if (!prepare_torrent_params_for_file(ex_info->torrent_params, path)) { 
		TCORE_WARNING("add torrent by path '%s' failed torrent file not valid", 
			path.string().c_str())
		return details::invalid_torrent_id;
	}

	if (!prepare_torrent_sandbox(ex_info->torrent_params)) {
		TCORE_WARNING("add torrent by path '%s' failed can not prepare torrent sandbox", 
			path.string().c_str())
		return details::invalid_torrent_id;
	}

	if (!params_.controller->add_torrent(ex_info->torrent_params, ex_info->handle)) 
		return details::invalid_torrent_id;
	
	ex_info->torrent_info.reset(new libtorrent::torrent_info(ex_info->handle.get_torrent_info()));	
	torrent_id = torrents_map_.size() + 1;
	torrents_map_[torrent_id] = ex_info;
	
	LIBTORRENT_EXCEPTION_SAFE_END_(return details::invalid_torrent_id)

	return torrent_id;	
}

int torrent_core::add_torrent_url(std::string const & url) 
{
	/** Setup torrent and envt. then async add new torrent by url to the core_session_ message queue. 
		NOTE if torrent already in queue, the torrent will be add by force */	
	details::torrent_ex_info ex_info;
	int torrent_id = details::invalid_torrent_id;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	if (cur_state_ != base_service::service_running) { 
		TCORE_WARNING("add torrent by path '%s' failed torrent core not runing", url.c_str())
		return invalid_torrent_id;
	}

	prepare_torrent_params_for_url(ex_info.torrent_params, url);

	LIBTORRENT_EXCEPTION_SAFE_END_(torrent_id = details::invalid_torrent_id)

	return torrent_id;
}

std::string torrent_core::get_torrent_info(int torrent_id) const 
{
	details::torrents_map_type::const_iterator found;

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id '%i' failed torrent core not runing", torrent_id)
		return std::string();
	}

	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	if ((found = torrents_map_.find(torrent_id)) != torrents_map_.end())  
		return details::torrent_info_to_json(found->second);

	LIBTORRENT_EXCEPTION_SAFE_END

	return std::string();
}

std::string torrent_core::start_torrent_download(int torrent_id, int file_id) 
{
	/** To start download just return notmal prior to req. file, 
		then post message to main loop about this change */
	TCORE_TRACE("start_torrent_download '%i' '%i'", torrent_id, file_id)

	details::torrents_map_type::iterator found;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN	
	
	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id '%i' failed torrent core not runing", torrent_id)
		return std::string();
	}

	if ((found = torrents_map_.find(torrent_id)) != torrents_map_.end()) {
		libtorrent::torrent_handle & handle = found->second->handle;
		if (found->second->torrent_info->num_files() > file_id) {
			handle.file_priority(file_id, details::file_ex_info::normal_prior);
			core_session_.post_torrent_updates();
			return found->second->torrent_info->file_at(file_id).path;	
		} // !if
	}

	LIBTORRENT_EXCEPTION_SAFE_END
	
	TCORE_WARNING("failed start download can not find torrent id '%i' or file id '%i'", 
		torrent_id, file_id)	
	
	return std::string();
}

void torrent_core::pause_download(int torrent_id, int file_id) 
{		
	/** To pause download just set off prior to req. file, 
		then post message to main loop about this change */
	details::torrents_map_type::iterator found;
	
	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id '%i' failed torrent core not runing", torrent_id)
		return;
	}

	if ((found = torrents_map_.find(torrent_id)) != torrents_map_.end()) {
		details::torrent_ex_info_ptr ex_info = found->second;
		if (found->second->torrent_info->num_files() > file_id) {
			ex_info->handle.file_priority(file_id, details::file_ex_info::off_prior);
			core_session_.post_torrent_updates();
		} // !if
	}

	LIBTORRENT_EXCEPTION_SAFE_END
}

void torrent_core::resume_download(int torrent_id, int file_id) 
{
	details::torrents_map_type::iterator found;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id '%i' failed torrent core not runing", torrent_id)
		return;
	}

	if ((found = torrents_map_.find(torrent_id)) != torrents_map_.end()) {
		details::torrent_ex_info_ptr ex_info = found->second;
		if (ex_info->handle.is_valid()) {
			if (file_id < 0) 
			{ 
				ex_info->handle.resume();
				core_session_.post_torrent_updates();
			}
			else if (found->second->torrent_info->num_files() > file_id) 
			{
				ex_info->handle.file_priority(file_id, details::file_ex_info::normal_prior);
				core_session_.post_torrent_updates();
			}
		} // !if
	}

	LIBTORRENT_EXCEPTION_SAFE_END
}	

void torrent_core::remove_torrent(int torrent_id) 
{
	details::torrents_map_type::iterator found;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id '%i' failed torrent core not runing", torrent_id)
		return;
	}
	
	if ((found = torrents_map_.find(torrent_id)) != torrents_map_.end()) {
		core_session_.remove_torrent(found->second->handle);
		torrents_map_.erase(found);
	}

	LIBTORRENT_EXCEPTION_SAFE_END
}

void torrent_core::stop_torrent_download(int torrent_id) 
{
	details::torrents_map_type::iterator found;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id '%i' failed torrent core not runing", torrent_id)
		return;
	}
	
	if ((found = torrents_map_.find(torrent_id)) != torrents_map_.end()) 
			found->second->handle.pause();
	
	LIBTORRENT_EXCEPTION_SAFE_END
}

/**
 * Private torrent_core api
 */

bool torrent_core::init_core_session() 
{
	/** Get & validate settings from t2h_core::settings_manager, 
		if all good & valid, then start the core_session_ async listen */
	if (init_torrent_core_settings()) 
	{
		boost::system::error_code error_code;
		core_session_.start_lsd();
		core_session_.start_upnp();
		core_session_.start_natpmp();
		core_session_.listen_on(std::make_pair(settings_.port_start, settings_.port_end), error_code);	
		setup_core_session();	
		return (error_code ? false : true);
	}
	return false;
}

void torrent_core::setup_core_session() 
{
	using libtorrent::session_settings;
	
	session_settings settings;	
	
	settings.user_agent = service_name();
	settings.choking_algorithm = session_settings::auto_expand_choker;
	settings.disk_cache_algorithm = session_settings::avoid_readback;
	settings.volatile_read_cache = false;

	params_.controller->on_setup_core_session(settings);
	core_session_.set_settings(settings);
}

bool torrent_core::init_torrent_core_settings() 
{
	try 
	{
		settings_.save_root = params_.setting_manager->get_value<std::string>("tc_root");
		settings_.port_start = params_.setting_manager->get_value<int>("tc_port_start");
		settings_.port_end = params_.setting_manager->get_value<int>("tc_port_end");
		settings_.max_alert_wait_time = params_.setting_manager->get_value<int>("tc_max_alert_wait_time");
	} 
	catch (setting_manager_exception const & expt) 
	{ 
		TCORE_ERROR("could not init torrent_core, with reason '%s'", expt.what())
		return false;
	}
	return true;
}

void torrent_core::core_main_loop()
{
	/** Main libtorrent(eg core_session_) loop, work in blocking mode, 
		if timeout came post the 'update' message to core_session_ message queue */
	TCORE_TRACE("entring to main notification loop")
	
	libtorrent::time_duration const wait_alert_time = libtorrent::seconds(settings_.max_alert_wait_time);
	while (cur_state_ == base_service::service_running) 
	{
		LIBTORRENT_EXCEPTION_SAFE_BEGIN
		if (core_session_.wait_for_alert(wait_alert_time) != NULL) {
			handle_core_notifications();
			continue;
		}
		core_session_.post_torrent_updates();
		LIBTORRENT_EXCEPTION_SAFE_END_(break)
	} // !loop
	cur_state_ = base_service::service_stoped;
}

void torrent_core::handle_core_notifications() 
{
	using libtorrent::alert;
	typedef std::deque<alert *> alerts_list_type;
	
	/** Dispatch libtorrent alerts(eg notifications) via abstract controller, 
		when free alert memory */
	alerts_list_type alerts;
	base_torrent_core_cntl_ptr controller = params_.controller;	
	core_session_.pop_alerts(&alerts);
	for (alerts_list_type::iterator it = alerts.begin(), end = alerts.end(); 
		it != end; 
		++it) 
	{
		try 
		{
			if (is_critical_error(*it) && 
				cur_state_ == base_service::service_running)
			{
				handle_critical_error_notification(*it);
			}
			controller->dispatch_alert(*it);
		}
		catch (std::exception const & expt) 
		{
			TCORE_WARNING("alert dispatching failed, with reason '%s'", expt.what())
		}	
		delete *it; *it = NULL;
	} // !for
}

bool torrent_core::is_critical_error(libtorrent::alert * alert) 
{
	using namespace libtorrent;

	if (alert->category() & alert::error_notification) { 
		int const type = alert->type();
		return (type == listen_failed_alert::alert_type) ? true : false;
	}

	return false;
}

void torrent_core::handle_critical_error_notification(libtorrent::alert * alert) 
{
	using namespace libtorrent;

	if (listen_failed_alert * listen_failed = alert_cast<listen_failed_alert>(alert)) {
		TCORE_ERROR("listen error notification came, with port '%i', error message '%s'", 
			listen_failed->endpoint.port(), listen_failed->error.message().c_str())
		cur_state_ = service_stoped;
	}
}

bool torrent_core::prepare_torrent_params_for_file(
	libtorrent::add_torrent_params & torrent_params, boost::filesystem::path const & path) 
{
	using namespace libtorrent;
	std::string const path_ = path.string();
	boost::system::error_code error_code;
	boost::intrusive_ptr<torrent_info> new_torrent_info = 
		new (std::nothrow) torrent_info(path_, error_code);	

	if (new_torrent_info && !error_code) {
		torrent_params.save_path = details::create_random_path(settings_.save_root);
		torrent_params.ti = new_torrent_info;
		torrent_params.flags |= add_torrent_params::flag_paused;
		torrent_params.flags &= ~add_torrent_params::flag_duplicate_is_error;
		torrent_params.flags |= add_torrent_params::flag_auto_managed;
		return true;
	}
	return false;
}

void torrent_core::prepare_torrent_params_for_url(
	libtorrent::add_torrent_params & torrent_params, std::string const & url) 
{
	using namespace libtorrent;

	torrent_params.save_path = details::create_random_path(settings_.save_root);
	torrent_params.flags |= add_torrent_params::flag_paused;
	torrent_params.flags &= ~add_torrent_params::flag_duplicate_is_error;
	torrent_params.flags |= add_torrent_params::flag_auto_managed;
	torrent_params.url = url;
}

bool torrent_core::prepare_torrent_sandbox(libtorrent::add_torrent_params & torrent_params) 
{
	try 
	{
		if (!boost::filesystem::exists(torrent_params.save_path))  
			return boost::filesystem::create_directory(torrent_params.save_path);
	} 
	catch (boost::filesystem3::filesystem_error const & expt) { /* do nothing */ }
	return false;
}

} // namespace t2h_core

