#ifndef HTTP_REQUEST_HPP_INCLUDED
#define HTTP_REQUEST_HPP_INCLUDED

#include "http_header.hpp"

/** 
 * A request received from a client
 */
namespace utility {

struct http_request {
	enum http_method_type { 
		mget = 0x1,
		mpost = 0x2,
		mhead = 0x3,
		munknown = mhead + 0x1
	}; 

	http_method_type mtype;
	std::string method;
	std::string uri;
	int http_version_major;
	int http_version_minor;
	header_list_type headers;
};

void http_request_set_mtype(http_request & req);

} // namespace utility

#endif 

