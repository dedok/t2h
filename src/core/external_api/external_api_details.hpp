#ifndef EXTERNAL_API_DETAILS_HPP_INCLUDED
#define EXTERNAL_API_DETAILS_HPP_INCLUDED

#include "services_manager.hpp"
#include "setting_manager.hpp"
#include "torrent_core.hpp"
#include "http_server_core.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core {

/**
 * Settings for the core_handle
 */
struct core_handle_settings {
	std::string config;			// config migth a path or a string(depends from config_load_from_file value)
	bool config_load_from_file; // true mean load from file, in this case config sould be a valid file path with JSON config inside.
};

/**
 * core_handle 
 */
class core_handle : public boost::noncopyable {
public :
	explicit core_handle(core_handle_settings const & settings);
	~core_handle();
	
	bool initialize();
	void destroy();
	void wait();
	
	inline torrent_core_ptr get_torrent_core() 
	{ 
		return boost::static_pointer_cast<torrent_core>(
				servs_manager_.get_service(torrent_core::this_service_name));
	}

	inline setting_manager_ptr get_setting_manager() 
		{ return sets_manager_; } 

private :
	void init_support_system();
	void init_core_services();
	
	torrent_core_ptr init_torrent_core();
	http_server_core_ptr init_http_server();

	core_handle_settings settings_;
	common::services_manager servs_manager_;
	setting_manager_ptr sets_manager_;

};

typedef boost::scoped_ptr<core_handle> core_handle_ptr;

} // namespace t2h_core


#endif

