#include "core_version.hpp"
#include "torrent_core.hpp"
#include "misc_utility.hpp"
#include "torrent_core_macros.hpp"
#include "torrent_core_utility.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/bitfield.hpp>

namespace t2h_core {

/**
 * Public torrent_core api
 */

char const * torrent_core::this_service_name = "torrent_core";

torrent_core::torrent_core(torrent_core_params const & params) : 
		common::base_service(this_service_name), 
		params_(params),
		cur_state_(base_service::service_state_unknown),
		settings_(),
		core_lock_(),
		shared_buffer_(NULL),
		core_session_(NULL),
		core_session_loop_()
{
}

torrent_core::~torrent_core() 
{
	stop_service();
}

bool torrent_core::launch_service() 
{
	try 
	{
		boost::lock_guard<boost::mutex> guard(core_lock_);
		
		if (cur_state_ == base_service::service_running) {
			TCORE_WARNING("fail to launch torrent core, torrent core already launced")
			return false;
		}
		
		core_session_ = new libtorrent::session(
								libtorrent::fingerprint("T2H", CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH, CORE_VERSION_BUILD), 
								libtorrent::session::add_default_plugins, 
								params_.controller->availables_categories());
		
		shared_buffer_ = new details::shared_buffer();

		params_.controller->set_session(core_session_);
		params_.controller->set_shared_buffer(shared_buffer_);

		if (!init_core_session()) {
			TCORE_WARNING("can not init torrent_core engine, settings not valid or ill formet")
			delete core_session_; core_session_ = NULL;
			delete shared_buffer_; shared_buffer_ = NULL;
			return false;
		}
		
		cur_state_ = base_service::service_running;
		core_session_loop_.reset(new boost::thread(&torrent_core::core_main_loop, this));
	}
	catch (libtorrent::libtorrent_exception const & expt) 
	{
		TCORE_ERROR("launching of the torrent core failed, with reason '%s'", expt.what())
		cur_state_ = base_service::service_stoped;
		return false;
	}

	return (cur_state_ == base_service::service_running);
}

void torrent_core::stop_service() 
{
	/** Stop core_session_ subsytems, then core_session_ main loop. 
		NOTE: To stop all 'trackers' session need to call dtor of core_session_ */
	boost::lock_guard<boost::mutex> guard(core_lock_);
	if (cur_state_ == base_service::service_running) {
		if (settings_.loadable_session) 
			s11z_session_state();

#ifndef TORRENT_DISABLE_DHT
		core_session_->stop_dht(); 
#endif
		core_session_->stop_lsd();
		core_session_->stop_upnp(); 
		core_session_->stop_natpmp();
		
		cur_state_ = base_service::service_stoped;
		core_session_->post_torrent_updates();	
		core_session_loop_->join();
		delete core_session_; core_session_ = NULL;
	}
}

void torrent_core::s11z_session_state() 
{
	std::vector<char> state_bytes;
	libtorrent::entry session_state_entry;
	core_session_->save_state(session_state_entry);
	libtorrent::bencode(std::back_inserter(state_bytes), session_state_entry);
	if (details::save_file(libtorrent::combine_path(settings_.save_root, ".ses_state"), 
		state_bytes) == -1 ) 
	{
		TCORE_WARNING("can not save torrent session state")	
	}
}

void torrent_core::wait_service() 
{
	if (cur_state_ == base_service::service_running)
		core_session_loop_->join();	
}

torrent_core::ptr_type torrent_core::clone() 
{
	boost::lock_guard<boost::mutex> guard(core_lock_);
	return ptr_type(new torrent_core(params_)); 
}

common::base_service::service_state torrent_core::get_service_state() const 
{
	boost::lock_guard<boost::mutex> guard(core_lock_);
	return cur_state_;
}

torrent_core::size_type torrent_core::add_torrent(boost::filesystem::path const & path) 
{
	/** Setup torrent and envt. then async add new torrent by file path to the '.torrent' file.
		Also adding extended info to shared_buffer.
		NOTE if torrent already in queue(core_session_), the torrent will not rewrited */	

	using libtorrent::torrent_info;
	
	bool add_state = false;
	std::size_t torrent_id = torrent_core::invalid_torrent_id;
	details::torrent_ex_info_ptr ex_info(new details::torrent_ex_info());
	
	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) { 
		TCORE_WARNING("add torrent by path '%s' failed torrent core not runing", 
			path.string().c_str())
		return torrent_core::invalid_torrent_id;
	}
			
	if (!details::torrent_ex_info::initialize_f(ex_info, 
		boost::filesystem::path(settings_.save_root), path)) 
	{
		TCORE_WARNING("add torrent by path '%s' failed can not initaliza torrent params", 
			path.string().c_str())
		return torrent_core::invalid_torrent_id;
	}	
	
	/*  Firstable we must to add into shared_buffer_ extended_info after do the real add operation. */
	boost::tie(add_state, torrent_id) = shared_buffer_->add(ex_info);
	if (!add_state) {
		TCORE_WARNING("add torrent by path '%s' failed can not add to buffer", 
			path.string().c_str())
		return torrent_core::invalid_torrent_id;
	}
	
	if (!params_.controller->add_torrent(ex_info)) { 
		shared_buffer_->remove(ex_info->handle.save_path());
		return torrent_core::invalid_torrent_id;
	}

	LIBTORRENT_EXCEPTION_SAFE_END_(return torrent_core::invalid_torrent_id)
	
	return torrent_id;	
}

torrent_core::size_type torrent_core::add_torrent_url(std::string const & url) 
{
	/** Setup torrent and envt. then async add new torrent by url.
		Also adding extended info to shared_buffer.
		NOTE if torrent already in queue(core_session_), the torrent will not rewrited */	
	details::torrent_ex_info ex_info;
	size_type torrent_id = invalid_torrent_id;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) { 
		TCORE_WARNING("add torrent by path '%s' failed torrent core not runing", url.c_str())
		return invalid_torrent_id;
	}
	
	// TODO impl this

	LIBTORRENT_EXCEPTION_SAFE_END_(torrent_id = torrent_core::invalid_torrent_id)

	return torrent_id;
}

std::string torrent_core::get_torrent_info(torrent_core::size_type torrent_id) const 
{
	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("get torrent info by id "SL_SIZE_T" failed torrent core not runing", torrent_id)
		return std::string();
	}

	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	details::torrent_ex_info_ptr ex_info = shared_buffer_->get(torrent_id);
	if (ex_info)
		return details::torrent_info_to_json(ex_info);

	LIBTORRENT_EXCEPTION_SAFE_END

	return std::string();
}

std::string torrent_core::start_torrent_download(torrent_core::size_type torrent_id, int file_id) 
{
	/** To start download just set to normal prior to req. file, 
		then post message to main loop about changes */
	LIBTORRENT_EXCEPTION_SAFE_BEGIN	
	
	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("start download by id "SL_SIZE_T" failed torrent core not runing", torrent_id)
		return std::string();
	}
	
	details::torrent_ex_info_ptr ex_info = shared_buffer_->get(torrent_id);
	if (ex_info) {
		libtorrent::torrent_info const & info = ex_info->handle.get_torrent_info();
		if (info.num_files() > file_id && file_id >= 0) {
			ex_info->handle.file_priority(file_id, details::file_ex_info::normal_prior);
			ex_info->handle.force_reannounce();	
			core_session_->post_torrent_updates();
			return libtorrent::combine_path(ex_info->sandbox_dir_name, info.file_at(file_id).path);
		} // if
	} // if

	LIBTORRENT_EXCEPTION_SAFE_END
	
	return std::string();
}

void torrent_core::pause_download(torrent_core::size_type torrent_id, int file_id) 
{		
	/** To pause download just set off prior to req. file, 
		then post message to main loop about this change */
	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("pause download by id "SL_SIZE_T" failed torrent core not runing", torrent_id)
		return;
	}
	
	details::torrent_ex_info_ptr ex_info = shared_buffer_->get(torrent_id);
	if (ex_info) {
		libtorrent::torrent_info const & info = ex_info->handle.get_torrent_info();
		if (info.num_files() > file_id && file_id >= 0) {
			ex_info->handle.file_priority(file_id, details::file_ex_info::off_prior);
			core_session_->post_torrent_updates();
		} // if
	} // if

	LIBTORRENT_EXCEPTION_SAFE_END
}

void torrent_core::resume_download(torrent_core::size_type torrent_id, int file_id) 
{
	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	boost::lock_guard<boost::mutex> guard(core_lock_);
	
	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("resume download by id "SL_SIZE_T" failed torrent core not runing", torrent_id)
		return;
	}
	
	details::torrent_ex_info_ptr ex_info = shared_buffer_->get(torrent_id);
	if (ex_info) {
		libtorrent::torrent_info const & native_info = ex_info->handle.get_torrent_info();
		ex_info->handle.resume();
		if (native_info.num_files() > file_id && file_id >= 0) {
			ex_info->handle.file_priority(file_id, details::file_ex_info::normal_prior);
			core_session_->post_torrent_updates();
		} // if
	} // if

	LIBTORRENT_EXCEPTION_SAFE_END
}	

void torrent_core::remove_torrent(size_type torrent_id) 
{
	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("remove download by id "SL_SIZE_T" failed torrent core not runing", torrent_id)
		return;
	}
	
	// TODO add saving resumed data
	details::torrent_ex_info_ptr ex_info = shared_buffer_->get(torrent_id);
	if (ex_info) {
		core_session_->remove_torrent(ex_info->handle);
		shared_buffer_->remove(torrent_id);
		core_session_->post_torrent_updates();
	}

	LIBTORRENT_EXCEPTION_SAFE_END
}

void torrent_core::stop_torrent_download(torrent_core::size_type torrent_id) 
{
	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	
	boost::lock_guard<boost::mutex> guard(core_lock_);

	if (cur_state_ != base_service::service_running) {
		TCORE_WARNING("stop download by id "SL_SIZE_T" failed torrent core not runing", torrent_id)
		return;
	}
	
	details::torrent_ex_info_ptr ex_info = shared_buffer_->get(torrent_id);
	if (ex_info) { 
		ex_info->handle.pause();
		core_session_->post_torrent_updates();
	}

	LIBTORRENT_EXCEPTION_SAFE_END
}

/**
 * Private torrent_core api
 */

bool torrent_core::init_core_session() 
{
	/** Get & validate settings from t2h_core::settings_manager, 
		if all good & valid, then start the core_session_ async listen */
	std::vector<char> bytes;
	bool has_prev_state = false;
	boost::system::error_code error_code;
	std::string const session_state_filename = std::string(".") + service_name() + std::string("_state");
		
	if (init_torrent_core_settings()) {
		if (settings_.loadable_session) {
			std::string const session_state_filepath = 
				libtorrent::combine_path(settings_.save_root, session_state_filename);
			if (libtorrent::load_file(session_state_filepath, bytes, error_code) == 0) {
				libtorrent::lazy_entry entry;
				if (libtorrent::lazy_bdecode(&bytes.at(0), &bytes.at(0) + bytes.size(), 
						entry, 
						error_code) 
					== 0) 
				{
					core_session_->load_state(entry);
					has_prev_state = true;
				} // lazy_bdecode
			} // loadable_session
		} else 
			return false;
			
		core_session_->start_lsd();
		core_session_->start_upnp();
		core_session_->start_natpmp();	
		
		if (!has_prev_state)	
			setup_core_session();	
		
		core_session_->listen_on(std::make_pair(settings_.port_start, settings_.port_end), error_code);	
		
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
	core_session_->set_settings(settings);
}

bool torrent_core::init_torrent_core_settings() 
{
	boost::system::error_code error;
	try 
	{
		boost::filesystem::path tc_root = params_.setting_manager->get_value<std::string>("tc_root");
		if (!boost::filesystem::exists(tc_root, error) && 
			boost::filesystem::is_directory(tc_root, error)) 
		{
			return false;
		}
		settings_.save_root = boost::filesystem::absolute(tc_root).string();
		settings_.port_start = params_.setting_manager->get_value<int>("tc_port_start");
		settings_.port_end = params_.setting_manager->get_value<int>("tc_port_end");
		settings_.max_alert_wait_time = params_.setting_manager->get_value<int>("tc_max_alert_wait_time");
		settings_.loadable_session = params_.setting_manager->get_value<bool>("tc_loadable_session");
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
		core_session_->post_torrent_updates();
		if (core_session_->wait_for_alert(wait_alert_time) != NULL) { 
			handle_core_notifications();
			continue;
		}
		LIBTORRENT_EXCEPTION_SAFE_END_(continue)
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
	base_torrent_core_cntl_ptr controller;	
	
	core_session_->pop_alerts(&alerts);
	controller = params_.controller;

	for (alerts_list_type::iterator it = alerts.begin(), end = alerts.end(); 
		it != end; 
		++it) 
	{
#if 0
		TCORE_TRACE("handle_core_notifications handling with notification '%i'", (*it)->type())
#endif
		TORRENT_TRY 
		{
			if (*it == NULL) { 
				TCORE_WARNING("handle_core_notification have not valid alert")
				continue;
			}
			(is_critical_error(*it)) ?
				handle_critical_error_notification(*it) :
				controller->dispatch_alert(*it);
		}
		TORRENT_CATCH (std::exception const & expt) 
		{
			TCORE_WARNING("alert dispatching failed, with reason '%s'", expt.what())
		}
		delete *it;
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

} // namespace t2h_core

