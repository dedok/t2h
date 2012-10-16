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

bool http_request_get_range_header(http_request const & req, http_header & header) 
{
	header_list_type::const_iterator it = req.headers.begin(), 
		end = req.headers.end(), range_header = req.headers.end(); 
	for (; it != end; ++it) {
		if (it->name == "Range") {
			header = *it;
			return true;
		} // !if
	} // !for
	return false;
}

} // namspace utility

