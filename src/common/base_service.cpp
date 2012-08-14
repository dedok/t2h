#include "base_service.hpp"

namespace common {

base_service::base_service(std::string const & name) 
	: service_name_(name) 
{ 
}

base_service::~base_service() 
{
}

std::string base_service::service_name() const 
{
	return service_name_;
}

} // namespace common

