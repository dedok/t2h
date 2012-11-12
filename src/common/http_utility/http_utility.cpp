#include "http_utility.hpp"

#include <sstream>
#include <boost/lexical_cast.hpp>

namespace utility {

/**
 *
 */
static boost::int64_t cast_string_range_to_int(std::string const & str, std::size_t first, std::size_t last) 
{
	boost::int64_t result = range_header::bad;
	std::string tmp_buf;
	try 
	{
		for (;first != last; ++first)
			tmp_buf.push_back(str.at(first));
		if (tmp_buf == "*" || tmp_buf.size() == 0)
			return range_header::all;
		result = boost::lexical_cast<boost::int64_t>(tmp_buf);
	} 
	catch (std::exception const &) 
	{
		return range_header::bad;	
	}
	return result;
}

/**
 *
 */

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

boost::tuple<bool, http_header> 
	http_get_header(header_list_type const & headers, std::string const & name) 
{
	for (header_list_type::const_iterator first = headers.begin(), last = headers.end(); 
		first != last; 
		++first) 
	{
		if (first->name == name)
			return boost::make_tuple(true, *first);
	}
	return boost::make_tuple(false, http_header());
}

bool http_range_header_value_parser(
		range_header & rheader, 
		std::string const & value, 
		std::string::size_type start_range, 
		std::string::size_type end_range,
		bool is_second_part) 
{
	std::string value_range; 
	std::string::size_type first = 0, range_delim_pos = 0;
	boost::int64_t bstart = range_header::bad, bend = range_header::bad;
	
	for(;start_range != end_range; ++start_range) 
		value_range.push_back(value.at(start_range));
	
	if ((range_delim_pos = value_range.find_first_of("-")) == std::string::npos) 
		return false;
	
	bstart = cast_string_range_to_int(value_range, first, range_delim_pos);	
	bend = cast_string_range_to_int(value_range, range_delim_pos + 1, value_range.size());
	
	if (bstart == range_header::bad || bend == range_header::bad) 
		return false;
	
	is_second_part ? rheader.bstart_2 = bstart : rheader.bstart_1 = bstart;
	is_second_part ? rheader.bend_2 = bend : rheader.bend_1 = bend;
	
	return true;
}

bool http_translate_range_header(range_header & rheader, header_list_type const & headers)
{
	http_header header;
	bool state = false;

	boost::tie(state, header) = http_get_header(headers, "Range");
	if (state) {
		std::string::size_type const end = header.value.size();
		std::string::size_type start = std::string::npos, delim_pos = std::string::npos;

		if ((start = header.value.find_first_of("=")) == std::string::npos)
			return false;

		if ((delim_pos = header.value.find_first_of(",")) != std::string::npos) { 
			if (delim_pos + 1 == end)
				return false;
			state = http_range_header_value_parser(rheader, header.value, delim_pos + 1, end, true);
			delim_pos = delim_pos;
		} else delim_pos = end;
		if (state) 
			return http_range_header_value_parser(rheader, header.value, start + 1, delim_pos, false);
	} else {
		boost::tie(state, header) = http_get_header(headers, "Accept");
		if (state) {
			// TODO fix this
			rheader.bstart_1 = rheader.bend_1 = range_header::all;
			return true;
		}
	} // if

	return false;
}

bool http_translate_range_header(range_header & rheader, char const * header) 
{
	rheader.bstart_1 = rheader.bend_1 = rheader.bstart_2 = rheader.bend_1 = 0;
#if defined (LC_USE_CSF_TRANSLATOR)
	return (sscanf(header, 
		"bytes=" LC_INTMAX_SF "-" LC_INTMAX_SF, 
		&rheader.bstart_1, &rheader.bend_1) > 0 ? true : false);
#else
#	warning [libcommon] utility::http_translate_range_header implementation not exist 
#endif // LIBCOMMON_USE_SSPRINTF_TRANSLATOR
	return false;
}

bool http_translate_accept_header(range_header & rheader, char const * header) 
{
	// TODO add parsing of data
	rheader.bstart_1 = rheader.bend_1 = range_header::all; 
	return true; 
}

} // namespace utility

