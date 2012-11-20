#include "send_content_reply.hpp"

#include "mime_types.hpp"
#include "http_server_utility.hpp"

namespace t2h_core { namespace details {

/**
 * Public send_content_reply api
 */

send_content_reply::send_content_reply() : http_core_reply() 
{ 

}

send_content_reply::~send_content_reply() 
{

}

boost::tuple<std::string, bool> send_content_reply::get_reply_headers(http_data & hd) 
{
	std::string const gmt_time = utility::http_get_gmt_time_string();
	
	/*  NOTE : Prepare Etag, Date, Last-Modified headers. Must be in UTC, according to 
	 	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.*/
	std::string reply_headers = "HTTP/1.1 200 Ok\r\n";
	add_etag_date_headers(reply_headers, hd.fi);
	reply_headers += "Content-Type: application/octet-stream\r\n";
	reply_headers += "Content-Length: " + boost::lexical_cast<std::string>(hd.fi->file_size);
	reply_headers += "\r\n\r\n";

	return boost::make_tuple(reply_headers, true);
}

} } // namespace t2h_core, details

