#ifndef SERVICES_MANAGER_HPP_INCLUDED
#define SERVICES_MANAGER_HPP_INCLUDED

#include "base_service.hpp"

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace common {

class services_manager {
public :	
	services_manager();
	~services_manager();

	bool registrate(base_service_ptr service);
	base_service_ptr unregistrate(std::string const & name);

	void stop_all();

	base_service::service_state get_service_state(std::string const & name) const;
	base_service_ptr get_service(std::string const & name);

private :
	typedef boost::unordered_map<std::string, base_service_ptr> services_list_type;

	services_list_type services_;
	boost::mutex mutable lock_;
};

} // namespace common

#endif

