#ifndef T2H_DETAILS_HPP_INCLUDED
#define T2H_DETAILS_HPP_INCLUDED

#include "services_manager.hpp"
#include "t2h_settings_manager.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core {

struct core_handle_settings {
	std::string config_path;
};

class core_handle : public boost::noncopyable {
public :
	explicit core_handle(core_handle_settings const & settings);
	~core_handle();
	
	bool initialization();
	bool destroy();

private :
	bool init_support_system();
	bool init_core_services();
	
	core_handle_settings settings_;
	common::services_manager servs_manager_;
	t2h_core::setting_manager_ptr sets_manager_;

};

typedef boost::scoped_ptr<core_handle> core_handle_ptr;

} // namespace t2h_core


#endif

