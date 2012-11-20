#include "partial_content_reply.hpp"

#include "mime_types.hpp"
#include "http_server_utility.hpp"

namespace t2h_core { namespace details {

/**
 * Public partial_content_reply api
 */

partial_content_reply::partial_content_reply() : http_core_reply() 
{

}

partial_content_reply::~partial_content_reply() 
{

}

boost::tuple<std::string, bool> partial_content_reply::get_reply_headers(http_data & hd) 
{
	boost::int64_t content_size = 1 * ((hd.read_end - hd.read_start) + 1);
	content_size = (content_size > hd.fi->file_size ? hd.fi->file_size : content_size);
	std::string const gmt_time = utility::http_get_gmt_time_string();
	boost::int64_t const end = hd.read_end == hd.fi->file_size ? hd.read_end - 1 : hd.read_end;
	
	/*  NOTE : Prepare Etag, Date, Last-Modified headers. Must be in UTC, according to 
	 	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.*/
	std::string reply_headers = "HTTP/1.1 206 Partial-Content\r\n";
	add_etag_date_headers(reply_headers, hd.fi);
	reply_headers +=	
		"Content-Type: application/octet-stream\r\n"
		"Accept-Ranges: bytes\r\n"	
		"Content-Range: ";
	reply_headers += "bytes " + boost::lexical_cast<std::string>(hd.read_start) + "-" +
		boost::lexical_cast<std::string>(end) + "/" +  boost::lexical_cast<std::string>(hd.fi->file_size);
	reply_headers += "\r\nContent-Length: " + boost::lexical_cast<std::string>(content_size);
	reply_headers += "\r\n\r\n";
	
	return boost::make_tuple(reply_headers, true);
}

} } // namesapce t2h_core, details

