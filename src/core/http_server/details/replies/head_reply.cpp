#include "head_reply.hpp"

#include "mime_types.hpp"
#include "http_server_utility.hpp"

namespace t2h_core { namespace details {

/**
 * Public head_reply api
 */

head_reply::head_reply() : http_core_reply() 
{
}

head_reply::~head_reply() 
{
}

boost::tuple<std::string, bool> head_reply::get_reply_headers(http_data & hd) 
{
	boost::int64_t const content_size = hd.fi->file_size;
	std::string const gmt_time = utility::http_get_gmt_time_string();
	
	/*  NOTE : Prepare Etag, Date, Last-Modified headers. Must be in UTC, according to 
	 	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.*/
	std::string reply_headers = "HTTP/1.1 200 Ok\r\n";
	add_etag_date_headers(reply_headers, hd.fi);
	reply_headers += "Content-Type: application/octet-stream\r\n";
	reply_headers += "Content-Length: " + boost::lexical_cast<std::string>(content_size);
	reply_headers += "\r\n\r\n";
	
	return boost::make_tuple(reply_headers, false);
}

} } // namespace t2h_core, details

