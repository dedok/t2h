#ifndef T2H_TORRENT_CORE_HPP_INCLUDED
#define T2H_TORRENT_CORE_HPP_INCLUDED

#include "base_service.hpp"
#include "t2h_torrent_core_config.hpp"
#include "t2h_settings_manager.hpp"
#include "base_torrent_core_cntl.hpp"

#pragma warning (push, 0)

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#pragma warning (pop)

#include <boost/filesystem/path.hpp>

namespace t2h_core {

struct torrent_core_params {
	setting_manager_ptr setting_manager;
	base_torrent_core_cntl_ptr controller;
};

class torrent_core : public common::base_service {
public :
	static int const invalid_torrent_id = -1;

	torrent_core(torrent_core_params const & params, std::string const & name);
	~torrent_core();
	
	void set_controller(base_torrent_core_cntl_ptr contreller);
	
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
		
	virtual ptr_type clone() { return ptr_type(); }

	int add_torrent(boost::filesystem::path const & path);

private :
	bool init_core_session();
	void setting_up_core_session();
	void core_loop();

	void handle_alerts();	
	
	torrent_core_params mutable params_;
	libtorrent::session core_session_;
	boost::scoped_ptr<boost::thread> thread_loop_;

};

} // namespace t2h_core

#endif

