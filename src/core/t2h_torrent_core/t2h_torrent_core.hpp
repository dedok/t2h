#ifndef T2H_TORRENT_CORE_HPP_INCLUDED
#define T2H_TORRENT_CORE_HPP_INCLUDED

#include "base_service.hpp"
#include "t2h_torrent_core_config.hpp"
#include "t2h_settings_manager.hpp"
#include "base_torrent_core_cntl.hpp"

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include <boost/thread.hpp>
#include <boost/unordered_set.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

struct torrent_ex_info 
	: public boost::enable_shared_from_this<torrent_ex_info> 
{
	enum init_state_ { not_valid = 0x0, not_init_by_core, valid };
	
	torrent_ex_info() 
		: id(0), 
		init_state(not_valid), 
		save_path(), 
		url_for_download(), 
		torrent_params() 
	{ 
	}
	
	torrent_ex_info(int id_) 
		: id(id_), init_state(not_valid) { }
	
	std::size_t id;
	init_state_ init_state;
	std::string save_path;	
	std::string url_for_download;
	libtorrent::add_torrent_params torrent_params;
};

static inline bool torrent_ex_info_less(torrent_ex_info const & a, torrent_ex_info const & b) 
{
	return (a.id < b.id);
}

static inline std::size_t torrent_ex_info_hash(torrent_ex_info const & a) 
{
	boost::hash<std::string> hasher;
	return hasher(a.save_path);
}

struct torrent_ex_info_hash_adapter {
	inline std::size_t operator()(torrent_ex_info const & a) const 
	{ 
		return torrent_ex_info_hash(a); 
	}
};

struct torrent_ex_info_less_adapter { 
	inline bool operator()(torrent_ex_info const & a, torrent_ex_info const & b) const 
	{
		return torrent_ex_info_less(a, b);
	}
};

typedef boost::unordered_set<torrent_ex_info, 
							torrent_ex_info_hash_adapter, 
							torrent_ex_info_less_adapter> torrents_info_set_type; 

/** torrent_core_settings is main settings for the t2h_core::torrent_core,
 	for better abstraction this object is hidden and for public access, 
	use t2h_core::setting_manager_ptr which passed via ctor of t2h_core::torrent_core 
	in t2h_core::torrent_core_params */
struct torrent_core_settings {
	std::string save_root;
	int port_start;
	int port_end;
	int max_alert_wait_time;
};

} // namespace details

/** The main torrent_core parameters, all filds of the t2h_core::torrent_core_params 
	must set to valid values, otherwise SEGFAULT ... */ 
struct torrent_core_params {
	setting_manager_ptr setting_manager;
	base_torrent_core_cntl_ptr controller;
};

/**
 * 
 */
class torrent_core : public common::base_service {
public :
	enum { invalid_torrent_id = -1 };
	static const char * this_service_name;

	torrent_core(torrent_core_params const & params);
	~torrent_core();
		
	/** Outside & manadgable via service_manager interface, not thread safe */
	
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
		
	virtual ptr_type clone();
	
	virtual common::base_service::service_state get_service_state() const;	
	
	/** Outside control interface, not thread safe */
	
	void set_controller(base_torrent_core_cntl_ptr controller);
	
	int add_torrent(boost::filesystem::path const & path);
	int add_torrent_url(std::string const & url);
	std::string start_torrent_download(int id);

	void pause_download(std::size_t torrent_id);
	void resume_download(std::size_t torrent_id);	
	void remove_torrent(std::size_t torrent_id);
	void stop_download(std::size_t torrent_id);
	
private :
	bool init_core_session();
	void setup_core_session();
	bool init_torrent_core_settings();
	
	void core_main_loop();
	void handle_core_notifications();	
	bool is_critical_error(libtorrent::alert * alert);
	void handle_critical_error_notification(libtorrent::alert * alert);
	
	bool prepare_torrent_params_for_file(
		libtorrent::add_torrent_params & torrent_params, boost::filesystem::path const & path);
	void prepare_torrent_params_for_url(
		libtorrent::add_torrent_params & torrent_params, std::string const & path);
	bool prepare_torrent_sandbox(libtorrent::add_torrent_params & torrent_params);
	
	torrent_core_params mutable params_;			
	base_service::service_state volatile mutable cur_state_;
	details::torrent_core_settings settings_;		
	boost::scoped_ptr<boost::thread> thread_loop_;
	details::torrents_info_set_type torrents_info_set_;
	libtorrent::session core_session_;

};

typedef boost::shared_ptr<torrent_core> torrent_core_ptr;

} // namespace t2h_core

#endif

