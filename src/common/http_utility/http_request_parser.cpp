#include "http_request_parser.hpp"

namespace utility {

/**
 * Helpers declaration & implementation
 */
namespace detail {

static inline bool is_char(int c)
{
	return (c >= 0 && c <= 127);
}

static inline bool is_ctl(int c)
{
	return (c >= 0 && c <= 31) || (c == 127);
}

static inline bool is_tspecial(int c)
{
	switch (c) {
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
		return true;
	default:
		return false;
	}
	return false;
}

static inline bool is_digit(int c)
{
	return (c >= '0' && c <= '9');
}

} // !detail

/**
 * Public http_request_parser api implementation
 */
http_request_parser::http_request_parser() : state_(method_start)
{
}

http_request_parser::~http_request_parser() 
{ 
}

http_request_parser::state http_request_parser::get_state() const 
{
	return state_;
}

void http_request_parser::reset_state()
{
	state_ = method_start;
}

/**
 * Private http_request_parser api implementation
 */
boost::tribool http_request_parser::consume(http_request & req, char input)
{
	using namespace detail;
	switch (state_) 
	{
	case method_start:
		if (!is_char(input) || 
			is_ctl(input) || 
			is_tspecial(input)) 
		{
			return false;
		}
		else 
		{
			state_ = method;
			req.method.push_back(input);
			return boost::indeterminate;
		}
	
	case method:
		if (input == ' ') 
		{
			state_ = uri;
			return boost::indeterminate;
		} 
		else 
		if (!is_char(input) || 
			is_ctl(input) || 
			is_tspecial(input))
		{
			return false;
		}
		else
		{
			req.method.push_back(input);
			return boost::indeterminate;
		}
		
	case uri_start:
		if (is_ctl(input))
			return false;
		else
		{
			state_ = uri;
			req.uri.push_back(input);
			return boost::indeterminate;
		}	
		
	case uri:
		if (input == ' ') 
		{
			state_ = http_version_h;
			return boost::indeterminate;
		}
		else if (is_ctl(input))
		{
			return false;
		}
		else
		{
			req.uri.push_back(input);
			return boost::indeterminate;
		}
		
	case http_version_h:
		if (input == 'H')
		{
			state_ = http_version_t_1;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
		
	case http_version_t_1:
		if (input == 'T')
		{
			state_ = http_version_t_2;
			return boost::indeterminate;
		}
		else
		{
    		return false;
		}
		
	case http_version_t_2:
		if (input == 'T')
		{
			state_ = http_version_p;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
		
	case http_version_p:
		if (input == 'P')
		{
			state_ = http_version_slash;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
		
	case http_version_slash:
		if (input == '/')
		{
			req.http_version_major = 0;
			req.http_version_minor = 0;
			state_ = http_version_major_start;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	
	case http_version_major_start:
		if (is_digit(input))
		{
			req.http_version_major = req.http_version_major * 10 + input - '0';
			state_ = http_version_major;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}

	case http_version_major:
		if (input == '.')
		{
			state_ = http_version_minor_start;
			return boost::indeterminate;
		}
		else if (is_digit(input))
		{
			req.http_version_major = req.http_version_major * 10 + input - '0';
			return boost::indeterminate;
		}
		else
		{
			return false;
		}

	case http_version_minor_start:
 		if (is_digit(input))
		{
			req.http_version_minor = req.http_version_minor * 10 + input - '0';
			state_ = http_version_minor;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}

	case http_version_minor:
		if (input == '\r')
		{
			state_ = expecting_newline_1;
			return boost::indeterminate;	
		}
		else if (is_digit(input))
		{
			req.http_version_minor = req.http_version_minor * 10 + input - '0';
			return boost::indeterminate;
		}
		else
		{
			return false;
		}

	case expecting_newline_1:
		if (input == '\n')
		{
			state_ = header_line_start;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}

	case header_line_start:
		if (input == '\r')
		{
			state_ = expecting_newline_3;
			return boost::indeterminate;
		}
		else if (!req.headers.empty() && 
			(input == ' ' || input == '\t'))
		{
			state_ = header_lws;
			return boost::indeterminate;
		}
		else if (!is_char(input) || 
				is_ctl(input) || 
				is_tspecial(input))
		{
			return false;
		}
		else
		{
			req.headers.push_back(http_header());
			req.headers.back().name.push_back(input);
			state_ = header_name;
			return boost::indeterminate;
		}

	case header_lws:
		if (input == '\r')
		{
			state_ = expecting_newline_2;
			return boost::indeterminate;
		}
		else if (input == ' ' || input == '\t')
		{
			return boost::indeterminate;
		}
		else if (is_ctl(input))
		{
			return false;
		}
		else
		{
			state_ = header_value;
			req.headers.back().value.push_back(input);
			return boost::indeterminate;
		}

	case header_name:
		if (input == ':')
		{
			state_ = space_before_header_value;
			return boost::indeterminate;
		}
		else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
		{
			return false;
		}
		else
		{
			req.headers.back().name.push_back(input);
			return boost::indeterminate;
		}

	case space_before_header_value:
		if (input == ' ')
		{
			state_ = header_value;
			return boost::indeterminate;
		}	
		else
		{
			return false;
		}

	case header_value:
		if (input == '\r')
		{
			state_ = expecting_newline_2;
			return boost::indeterminate;
		}
		else if (is_ctl(input))
		{
			return false;
		}
		else
		{
			req.headers.back().value.push_back(input);
			return boost::indeterminate;
		}

	case expecting_newline_2:
		if (input == '\n')
		{
			state_ = header_line_start;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}

	case expecting_newline_3:
		return (input == '\n');
	
	case state_type_max:
		return false;

	default:
    	return false;
	
	} // !switch

	return false;
} // !consume


} // !utlitity

