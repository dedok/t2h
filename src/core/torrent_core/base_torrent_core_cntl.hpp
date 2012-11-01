#ifndef BASE_TORRENT_CORE_CNTL_HPP_INCLUDED
#define BASE_TORRENT_CORE_CNTL_HPP_INCLUDED

#include "torrent_info.hpp"
#include "shared_buffer.hpp"
#include "setting_manager.hpp"
#include "torrent_core_event_handler.hpp"

#if defined(__GNUG__)
#	pragma GCC system_header
#endif

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core {

/**
 *
 */
struct torrent_udata {
	int index;
};

/**
 *
 */
class base_torrent_core_cntl : boost::noncopyable {
public :
	typedef boost::shared_ptr<base_torrent_core_cntl> ptr_type;

	base_torrent_core_cntl() { }
	virtual ~base_torrent_core_cntl() { }
	
	virtual void set_event_handler(torrent_core_event_handler_ptr event_handler) = 0;
	virtual void set_setting_manager(setting_manager_ptr sets_manager) = 0;
	virtual bool set_session(libtorrent::session * session_ref) = 0;
	
	virtual int availables_categories() const = 0;
	virtual void on_setup_core_session(libtorrent::session_settings & settings) = 0;

	virtual void set_shared_buffer(details::shared_buffer * buffer) = 0;

	virtual bool add_torrent(details::torrent_ex_info_ptr ex_info) = 0;
	
	virtual void dispatch_alert(libtorrent::alert * alert) = 0;
	virtual bool handle_with_critical_errors() { return false; }

private :

};

typedef base_torrent_core_cntl::ptr_type base_torrent_core_cntl_ptr;

} // namespace

#endif

