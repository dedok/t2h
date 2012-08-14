#ifndef HTTP_HEADER_HPP_INCLUDED
#define HTTP_HEADER_HPP_INCLUDED

#include <string>
#include <vector>

namespace utility {

struct http_header
{
	std::string name;
	std::string value;
};

typedef std::vector<http_header> header_list_type; 

} // namespace utility

#endif

