#include "services_manager.hpp"

namespace common {

services_manager::services_manager() : services_() 
{

}

services_manager::~services_manager() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
}

bool services_manager::registrate(base_service_ptr service)
{
	boost::lock_guard<boost::mutex> guard(lock_);
	if (service) {
		if (services_.find(service->service_name()) == services_.end())
			services_[service->service_name()] = service;
		return true;
	}
	return false;
}

base_service_ptr services_manager::unregistrate(std::string const & name)
{
	boost::lock_guard<boost::mutex> guard(lock_);
	base_service_ptr service;
	services_list_type::iterator found = services_.find(name);
	if (found != services_.end()) {
		service = found->second;
		services_.erase(found);
	}
	return service;
}

void services_manager::stop_all() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	for (services_list_type::const_iterator it = services_.begin(), 
		end = services_.end();
		it != end; 
		++it) 
	{
		if (it->second->get_service_state() == base_service::service_running) {
			it->second->stop_service(); 
			it->second->wait_service();
		}
	}
	services_.clear();
}

void services_manager::wait_all() 
{
	for (services_list_type::const_iterator it = services_.begin(), 
		end = services_.end();
		it != end; 
		++it) 
	{
		if (it->second->get_service_state() == base_service::service_running) 
			it->second->wait_service();
	}

}

base_service::service_state services_manager::get_service_state(std::string const & name) const
{
	boost::lock_guard<boost::mutex> guard(lock_);
	base_service::service_state state = base_service::service_state_unknown;
	services_list_type::const_iterator found = services_.find(name);
	if (found != services_.end()) 
		state = found->second->get_service_state();
	return state;
}

base_service_ptr services_manager::get_service(std::string const & name) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	services_list_type::const_iterator found = services_.find(name);
	return found != services_.end() ? found->second : base_service_ptr();
}

} // namespace common

