#ifndef HTTP_UTILITY_HPP_INCLUDED
#define HTTP_UTILITY_HPP_INCLUDED

#include "http_reply.hpp"          
#include "mime_types.hpp"
#include "http_header.hpp"         
#include "http_request.hpp"        
#include "http_request_parser.hpp" 

#include <boost/cstdint.hpp>

#define LC_INTMAX_SF				"%Ld"
#define LC_INTMAX_PF				"%ld"

namespace utility {

struct range_header 
{
	enum { 
		all = -2, 
		bad = -1 
	};
	
	boost::int64_t bstart_1;
	boost::int64_t bend_1;

	boost::int64_t bstart_2;
	boost::int64_t bend_2;
};

bool url_decode(std::string const & in, std::string & out);

boost::tuple<bool, http_header> http_get_header(std::string const & name);

bool http_translate_range_header(range_header & rheader, header_list_type const & headers);

bool http_translate_range_header(range_header & rheader, char const * range_header);

bool http_translate_accept_header(range_header & rheader, char const * range_header);

}

#endif

