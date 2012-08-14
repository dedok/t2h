#ifndef HTTP_REQUEST_HPP_INCLUDED
#define HTTP_REQUEST_HPP_INCLUDED

#include "http_header.hpp"

/** 
 * A request received from a client
 */
namespace utility {

struct http_request {
	std::string method;
	std::string uri;
	int http_version_major;
	int http_version_minor;
	header_list_type headers;
};

bool http_request_get_range_header(http_request const & req, http_header & header);

} // namespace utility

#endif 

