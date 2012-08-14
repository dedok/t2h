#include "services_manager.hpp"

namespace common {

services_manager::services_manager() : services_() 
{

}

services_manager::~services_manager() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
}

void services_manager::registrate(base_service_ptr service)
{
	boost::lock_guard<boost::mutex> guard(lock_);

}

base_service_ptr services_manager::unregistrate(std::string const & name)
{
	boost::lock_guard<boost::mutex> guard(lock_);
	base_service_ptr service;
	return service;
}

void services_manager::stop_all() 
{
	boost::lock_guard<boost::mutex> guard(lock_);

}

services_manager::service_state services_manager::get_service_state(std::string const & name) const
{
	boost::lock_guard<boost::mutex> guard(lock_);
	service_state state;
	return state;
}

base_service_ptr services_manager::get_service(std::string const & name) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	base_service_ptr service;
	return service;
}


} // namespace common

