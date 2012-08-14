#ifndef HTTP_REQUEST_PARSER_HPP_INCLUDED
#define HTTP_REQUEST_PARSER_HPP_INCLUDED

#include <boost/tuple/tuple.hpp>
#include <boost/logic/tribool.hpp>

namespace utility {

struct http_request;

/**
 * Http protocol parser.
 */
class http_request_parser {
public:
	/** Http state values */
	enum state {
		method_start = 0x1,
		method,
		uri_start,
		uri,
 		http_version_h,
		http_version_t_1,
		http_version_t_2,
		http_version_p,
		http_version_slash,
		http_version_major_start,
		http_version_major,
		http_version_minor_start,
		http_version_minor,
		expecting_newline_1,
		header_line_start,
		header_lws,
		header_name,
		space_before_header_value,
		header_value,
		expecting_newline_2,
		expecting_newline_3,
		state_type_max = expecting_newline_3 + 1
	};

	http_request_parser();
	~http_request_parser();

	/** Get current state */
	state get_state() const;
	
	/** Set the state to the method_start value */
	void reset_state();

	/** 
	 * Parse input data. The tribool return value is true when a complete request
	 * has been parsed, false if the data is invalid, indeterminate when more
	 * data is required. The InputIterator return value indicates how much of the
	 * input has been consumed. 
	 */
	template <typename InputIterator>
	boost::tuple<boost::tribool, InputIterator> parse(
						http_request & req, 
						InputIterator begin, 
						InputIterator end)
	{
		boost::tribool result = boost::indeterminate;
		while (begin != end) {
			result = consume(req, *begin++);
			if (result || !result)
				return boost::make_tuple(result, begin);	
		}
		return boost::make_tuple(boost::tribool(boost::indeterminate), begin);
	}

private:
	/** Consume each input character and fill http_request */
	boost::tribool consume(http_request & req, char input);

	/** The current state of the parser. */
	state state_;

};

} // !utility

#endif 

