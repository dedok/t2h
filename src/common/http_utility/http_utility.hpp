#ifndef HTTP_UTILITY_HPP_INCLUDED
#define HTTP_UTILITY_HPP_INCLUDED

#include "http_reply.hpp"          
#include "mime_types.hpp"
#include "http_header.hpp"         
#include "http_request.hpp"        
#include "http_request_parser.hpp" 

#include <ctime>
#include <boost/cstdint.hpp>

#define LC_INTMAX_SF				"%Ld"
#define LC_INTMAX_PF				"%ld"

namespace utility {

/**
 * Range header as data struct representation.
 */
struct range_header 
{
	enum { 
		all = -2, 		// For start means 0 for end means file size
		bad = -1 		// This value set when parsing failed(not valid requst)
	};
	
	boost::int64_t bstart_1;
	boost::int64_t bend_1;

	boost::int64_t bstart_2;
	boost::int64_t bend_2;
};

/**
 * http helper functions
 */

bool url_decode(std::string const & in, std::string & out);

boost::tuple<bool, http_header> http_get_header(std::string const & name);

bool http_translate_range_header(range_header & rheader, header_list_type const & headers);

bool http_translate_range_header(range_header & rheader, char const * range_header);

bool http_translate_accept_header(range_header & rheader, char const * range_header);

std::string http_normalize_uri(std::string const & uri); 

static inline std::string http_normalize_uri_c(char const * uri) 
	{ return http_normalize_uri(std::string(uri)); } 

std::string http_get_gmt_time_string(); 

std::string http_etag(std::string const & file_path);

std::string http_etag(boost::int64_t file_size, std::time_t const & last_write_time);

} // namespace utility

#endif

