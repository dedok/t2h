#include "t2h_torrent_core.hpp"

#include "misc_utility.hpp"

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

torrent_core::torrent_core(
	torrent_core_params const & params, std::string const & name) 
		: common::base_service(name), 
		params_(params),
		is_running_(false),
		settings_(),
		thread_loop_(),
		torrents_list_(),
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
		
		if ((is_running_ = init_core_session())) 
			thread_loop_.reset(new boost::thread(&torrent_core::core_main_loop, this));
	}
	catch (libtorrent::libtorrent_exception const & expt) 
	{
		TCORE_ERROR("launching of the torrent core failed, with reason '%s'", expt.what())
		return false;
	}

	return is_running_;
}

void torrent_core::stop_service() 
{
	if (is_running_) { 
		core_session_.post_torrent_updates();
		is_running_ = false;
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
	return ptr_type(new torrent_core(params_, base_service::service_name())); 
}

boost::tuple<int, boost::filesystem::path> 
	torrent_core::add_torrent(boost::filesystem::path const & path) 
{
	using namespace libtorrent;
	
	/** Setup torrent and envt. then async add new torrent to the core_session_ message queue. 
	 	NOTE if torrent already in queue, the torrent will be add by force */
	boost::system::error_code error_code;	
	add_torrent_params torrent_params; 
	
	if (!is_running_) 
		return boost::make_tuple((int)invalid_torrent_id, boost::filesystem::path(""));
	
	if (prepare_torrent_params(torrent_params, path) && 
		prepare_torrent_sandbox(torrent_params)) 
	{
		core_session_.async_add_torrent(torrent_params);
		
		boost::lock_guard<boost::mutex> guard(torrents_list_.lock);	
		details::torrent_handle_ex::hash_func_type hash_func;
		details::torrent_handle_ex handle_ex = 
			{ torrent_params.save_path, details::in_process, 
				libtorrent::torrent_handle(), hash_func(torrent_params.save_path) }; 

		torrents_list_.list[handle_ex.hash_path] = handle_ex;		
		core_session_.post_torrent_updates();

		return boost::make_tuple(handle_ex.hash_path, boost::ref(torrent_params.save_path));
	}

	return boost::make_tuple((int)invalid_torrent_id, boost::filesystem::path(""));
}

void torrent_core::pause_torrent(std::size_t torrent_id) 
{
	boost::lock_guard<boost::mutex> guard(torrents_list_.lock);
	details::tl_iterator found;
	if ((found = torrents_list_.list.find(torrent_id)) != 
		torrents_list_.list.end()) 
	{
		details::torrent_handle_ex & handle = found->second;
		handle.status = details::request_to_pause;
		core_session_.post_torrent_updates();
	}
}

void torrent_core::resume_download(std::size_t torrent_id) 
{
	boost::lock_guard<boost::mutex> guard(torrents_list_.lock);
	details::tl_iterator found;
	if ((found = torrents_list_.list.find(torrent_id)) != 
		torrents_list_.list.end()) 
	{
		details::torrent_handle_ex & handle = found->second;
		handle.status = details::request_to_resume;
		core_session_.post_torrent_updates();
	}
}	

void torrent_core::remove_torrent(std::size_t torrent_id) 
{
	boost::lock_guard<boost::mutex> guard(torrents_list_.lock);
	details::tl_iterator found;
	stop_download_unsafe(torrent_id);
	if ((found = torrents_list_.list.find(torrent_id)) != 
		torrents_list_.list.end()) 
	{
		details::torrent_handle_ex & handle = found->second;
		if (handle.lt_handle.is_valid())  
			core_session_.remove_torrent(handle.lt_handle);
		torrents_list_.list.erase(found);
		core_session_.post_torrent_updates();
	}
}

void torrent_core::stop_download(std::size_t torrent_id) 
{
	boost::lock_guard<boost::mutex> guard(torrents_list_.lock);
	stop_download_unsafe(torrent_id);
	core_session_.post_torrent_updates();
}

/**
 * Private torrent_core api
 */

bool torrent_core::init_core_session() 
{
	/** Get & validate settings from t2h_core::settings_manager, 
		if all good & valid, then start the core_session_ async listen */
 	if (init_torrent_core_settings()) {
		boost::system::error_code error_code;
		core_session_.listen_on(
			std::make_pair(settings_.port_start, settings_.port_end), error_code);	
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
	libtorrent::time_duration const wait_alert_time = 
		libtorrent::seconds(settings_.max_alert_wait_time);
	while(is_running_) {
		if (core_session_.wait_for_alert(wait_alert_time) != NULL) {
			handle_core_notifications();
			continue;
		}
		core_session_.post_torrent_updates();
	} // !loop
	boost::lock_guard<boost::mutex> guard(torrents_list_.lock);
	torrents_list_.list.clear();
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
			if (is_critical_error(*it) && is_running_) 
				handle_critical_error_notification(*it);
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
		return (type == listen_failed_alert::alert_type) 
			? true : false;
	}
	return false;
}

void torrent_core::handle_critical_error_notification(libtorrent::alert * alert) 
{
	using namespace libtorrent;
	if (listen_failed_alert * listen_failed = alert_cast<listen_failed_alert>(alert)) {
		TCORE_ERROR("listen error notification came, with port '%i', error message '%s'", 
			listen_failed->endpoint.port(), listen_failed->error.message().c_str())
		is_running_ = false;	
	}
}

bool torrent_core::prepare_torrent_params(
	libtorrent::add_torrent_params & torrent_params, boost::filesystem::path const & path) 
{
	using namespace libtorrent;

	boost::system::error_code error_code;
	boost::intrusive_ptr<torrent_info> new_torrent_info = 
		new (std::nothrow) torrent_info(path.c_str(), error_code);	

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

bool torrent_core::prepare_torrent_sandbox(libtorrent::add_torrent_params & torrent_params) 
{
	boost::system::error_code error;
	if (!boost::filesystem::exists(torrent_params.save_path, error) && 
		!boost::filesystem::is_directory(torrent_params.save_path)) 
	{ 
		return boost::filesystem::create_directory(torrent_params.save_path);
	}
	return true;
}

void torrent_core::stop_download_unsafe(std::size_t torrent_id) 
{
	details::tl_iterator found;
	if ((found = torrents_list_.list.find(torrent_id)) != 
		 torrents_list_.list.end()) 
	{ 
		details::torrent_handle_ex & handle = found->second;	
		handle.status = details::request_to_stop;
	}
}

}// namespace t2h_core

