#include "base_transport.hpp"

namespace common {

/**
 * Public http_server_exception api
 */
transport_exception::transport_exception(std::string const & message) 
	: std::exception(), message_(message) 
{ 
}

transport_exception::~transport_exception() throw() 
{ 
}

char const * transport_exception::what() const throw()
{ 
	return message_.c_str(); 
}

/**
 *  Public base_http_server api
 */

base_transport::base_transport(
	transport_config const & config) 
{ 
}

base_transport::~base_transport() 
{
}

} // namespace common

