#ifndef HTTP_UTILITY_HPP_INCLUDED
#define HTTP_UTILITY_HPP_INCLUDED

#include "http_reply.hpp"          
#include "mime_types.hpp"
#include "http_header.hpp"         
#include "http_request.hpp"        
#include "http_request_parser.hpp" 

namespace utility {

bool url_decode(std::string const & in, std::string & out);
boost::tuple<boost::int64_t, boost::int64_t, bool> parse_range_header(http_header const & header);

}

#endif

