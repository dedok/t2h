#ifndef BASE_TORRENT_CORE_CNTL_HPP_INCLUDED
#define BASE_TORRENT_CORE_CNTL_HPP_INCLUDED

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core {

class base_torrent_core_cntl : boost::noncopyable {
public :
	typedef boost::shared_ptr<base_torrent_core_cntl> ptr_type;

	explicit base_torrent_core_cntl() { }
	virtual ~base_torrent_core_cntl() { }
	
	virtual int availables_categories() const = 0;
	virtual void set_core_session(libtorrent::session * session_ref) = 0;
	virtual void on_setup_core_session(libtorrent::session_settings & settings) = 0;
	
	virtual void dispatch_alert(libtorrent::alert * alert) = 0;
	
	virtual bool handle_with_critical_errors() { return false; }

	virtual void post_pause_download(std::string const & torrent_name) = 0;
	virtual void post_resume_download(std::string const & torrent_name) = 0;
	virtual void post_stop_download(std::string const & torrent_name) = 0;
	virtual void post_remove_torrent(std::string const & torrent_name) = 0;

	virtual std::size_t decode_id(std::string const & torrent_name) = 0;
	virtual std::string encode_id(std::size_t torrent_id) = 0;
};

typedef base_torrent_core_cntl::ptr_type base_torrent_core_cntl_ptr;

} // namespace

#endif

