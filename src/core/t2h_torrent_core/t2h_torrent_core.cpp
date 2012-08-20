#include "t2h_torrent_core.hpp"

#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>

static void default_setup_torrent_params(
			libtorrent::add_torrent_params & params, 
			boost::intrusive_ptr<libtorrent::torrent_info> info, 
			std::string const & save_path) 
{
	using namespace libtorrent;
	params.save_path = save_path;
	params.ti = info;
	params.flags |= add_torrent_params::flag_paused;
	params.flags &= ~add_torrent_params::flag_duplicate_is_error;
	params.flags |= add_torrent_params::flag_auto_managed;
}

namespace t2h_core {

torrent_core::torrent_core(
	torrent_core_params const & params, std::string const & name) 
		: common::base_service(name), 
		params_(params), 
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
	using namespace libtorrent;
	bool state = false;
	TCORE_TRACE("init & lauching torrent core")
	try 
	{
		if (!thread_loop_) {
			if ((state = init_core_session())) 
			{
				thread_loop_.reset(new 
					boost::thread(&torrent_core::core_loop, this));
			}
		}
	}
	catch (libtorrent_exception const & expt) 
	{
		TCORE_ERROR("launching of the torrent core failed, with reason '%s'", expt.what())
		return false;
	}
	return state;
}

void torrent_core::stop_service() 
{
	// TODO impl this
	if (!thread_loop_) return;
	wait_service();
}

void torrent_core::wait_service() 
{
	if (!thread_loop_) return;
	thread_loop_->join();
}

int torrent_core::add_torrent(boost::filesystem::path const & path) 
{
	using namespace libtorrent;
	
	int id = invalid_torrent_id;
	boost::system::error_code error_code;	
	add_torrent_params torrent_params; 
	boost::intrusive_ptr<torrent_info> tor_info = 
		new (std::nothrow) torrent_info(path.c_str(), error_code);
	
	if (tor_info && !error_code) {
		TCORE_TRACE("adding new torrent by path '%s'", path.c_str())
		default_setup_torrent_params(torrent_params, tor_info, "./");
		core_session_.async_add_torrent(torrent_params);
	} // !if
	
	return id;
}

/**
 * Private torrent_core api
 */

bool torrent_core::init_core_session() 
{
	boost::system::error_code error_code;
	std::size_t port_start = params_.setting_manager->get_value<std::size_t>("port_start"), 
		port_end = params_.setting_manager->get_value<std::size_t>("port_end");
	
	core_session_.listen_on(std::make_pair(port_start, port_end), error_code);	
	setting_up_core_session();

	return (error_code ? false : true);
}

void torrent_core::setting_up_core_session() 
{
	using namespace libtorrent;
	session_settings settings;
	
	settings.user_agent = service_name();
	settings.choking_algorithm = session_settings::auto_expand_choker;
	settings.disk_cache_algorithm = session_settings::avoid_readback;
	settings.volatile_read_cache = false;

	core_session_.set_settings(settings);
}

void torrent_core::core_loop()
{
	/** main libtorrent(eg core) loop, work in blocking mode */
	std::size_t const max_wait_time 
		= params_.setting_manager->get_value<std::size_t>("max_wait_time");
	for (libtorrent::time_duration const max_wait = 
		libtorrent::seconds(max_wait_time);;
		) 
	{
		if (core_session_.wait_for_alert(max_wait) != NULL)
			handle_alerts();
	} // !loop
}

void torrent_core::handle_alerts() 
{
	using namespace libtorrent;
	typedef std::deque<alert *> alerts_list_type;
	
	/** dispatch libtorrent alerts(eg notifications) 
		via abstract controller, when free alert memory */
	alerts_list_type alerts;
	base_torrent_core_cntl_ptr controller = params_.controller;	
	core_session_.pop_alerts(&alerts);
	for (alerts_list_type::iterator it = alerts.begin(), end = alerts.end(); 
		it != end; 
		++it) 
	{
		try 
		{
			controller->dispatch_alert(*it);
		}
		catch (std::exception const & expt) 
		{
			TCORE_WARNING("alert dispatching failed, with reason '%s'", expt.what())
		}	
		delete *it;
	} // !for
	alerts.clear();
}

}// namespace t2h_core

