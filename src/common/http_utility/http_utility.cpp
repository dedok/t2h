#include "http_utility.hpp"

#include <sstream>

namespace utility {

static boost::int64_t cast_string_range_to_int(std::string const & str, std::size_t first, std::size_t last) 
{
	boost::int64_t result = -1;
	std::string tmp_buf;
	try 
	{
		for (;first != last; ++first)
			tmp_buf.push_back(str.at(first));
		result = atoi(tmp_buf.c_str());
	} 
	catch (std::exception const &) 
	{
		return -1;	
	}
	return result;
}

bool url_decode(std::string const & in, std::string & out)
{
	std::size_t const in_size = in.size();
	
	out.clear();
	out.reserve(in_size);
	
	for (std::size_t i = 0; 
		i < in_size; 
		++i) 
	{
		if (in[i] == '%') {
			if (i + 3 <= in_size) {
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value) {
					out += static_cast<char>(value);
					i += 2;
				} else return false;
			} else return false;	
		} else if (in[i] == '+') {
			out += ' ';
		} else {
			out += in[i];
		}
	} // !for
	
	return true;
}

boost::tuple<boost::int64_t, boost::int64_t, bool> parse_range_header(http_header const & header) 
{
	boost::tuple<boost::int64_t, boost::int64_t, bool> result(0, -1, false);

	std::size_t first = std::string::npos, 
		last = std::string::npos, end = header.value.size();
	
	if ((first = header.value.find_first_of("=")) == std::string::npos)
		return result;
	if ((last = header.value.find_last_of("-")) == std::string::npos)
		return result;
	
	result.get<0>() = cast_string_range_to_int(header.value, first + 1, last);	
	result.get<1>() = cast_string_range_to_int(header.value, last + 1, end);
	result.get<2>() = true;

	return result;
}

} // namespace utility

