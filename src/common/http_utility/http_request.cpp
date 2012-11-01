#include "http_request.hpp"

namespace utility {

void http_request_set_mtype(http_request & req) 
{
	if (req.method == "GET")
		req.mtype = http_request::mget;
	else if (req.method == "POST")
		req.mtype = http_request::mpost;
	else if (req.method == "HEAD")
		req.mtype = http_request::mhead;
	else
		req.mtype = http_request::munknown;
}

} // namspace utility

