#include "http_reply.hpp"
#include "http_reply_resources.hpp"
#include "misc_utility.hpp"

#include <boost/bind.hpp>

/**
 * Private hidden http_reply helpers
 */

namespace utility {

/**
 * Public http_reply api
 */
http_reply::http_reply(http_reply::buffer_type & buffer) 
	: buf_ref_(buffer), 
	enable_buf_realocation_(true)
{
	if (!buf_ref_.empty())
		buf_ref_.clear();
} 

http_reply::~http_reply() 
{

}

bool http_reply::stock_reply(http_reply::status_type status) 
{
	try 
	{
		reset_buffer();
		std::string const content = stock_replies::cast_to_string(status);
		std::size_t const content_size = content.size();
		add_status(status);
		add_header("Content-Length", safe_lexical_cast<std::string>(content_size));
		add_header("Content-Type", "text/html");
		add_content(content.c_str(), content_size);
	} 
	catch (std::exception const & expt) 
	{
		return false;
	}
	return true;
} 

bool http_reply::add_status(http_reply::status_type status) 
{
	try 
	{
		status_strings::cast_to_buffer(status, buf_ref_);
	}
	catch (std::exception const & expt) 
	{
		return false;
	}
	return true;
}

bool http_reply::add_header(std::string const & name, std::string const & value) 
{
	try 
	{
		add_header_directly(name, value);
		add_crlf();
	}
	catch (std::exception const & expt)
	{
		return false;
	}
	return true;
}

bool http_reply::add_header(http_header const & header) 
{
	try 
	{
		add_header_directly(header);
		add_crlf();
	}
	catch (std::exception const & expt)
	{
		return false;
	}
	return true;
}

bool http_reply::add_content(char const * content, std::size_t content_size) 
{
	if (!content || content_size == 0) 
		return false;	
	
	try 
	{
		add_crlf(); 
		std::size_t const prev_buf_size = buf_ref_.size();
		std::size_t new_buf_size = prev_buf_size + content_size;
		
		if (enable_buf_realocation_)
			buf_ref_.resize(new_buf_size);
		
		for (std::size_t it = prev_buf_size, it_ = 0; 
			it < new_buf_size && it_ < content_size; 
			++it, ++it_) 
		{
			buf_ref_.at(it) = content[it_];
		}
	} 
	catch (std::exception const &) 
	{
		reset_buffer();
		return false;	
	} 
	return true;
}	

void http_reply::add_crlf() 
{
	std::copy(misc_strings::crlf.begin(), misc_strings::crlf.end(), 
		std::back_inserter(buf_ref_));
}

void http_reply::enable_buf_realocation(bool state) 
{
	enable_buf_realocation_ = state;
}

void http_reply::reset_buffer() 
{
	buf_ref_.clear();
}

/**
 * Private http_reply api
 */

void http_reply::add_header_directly(http_header const & header) 
{
	add_header_directly(header.name, header.value);
}

void http_reply::add_header_directly(std::string const & name, std::string const & value) 
{
	std::size_t const misc_strings_size = misc_strings::name_value_separator.size();
	std::size_t prev_buf_size = buf_ref_.size();
	std::size_t new_buf_size = prev_buf_size + name.size() + value.size() + misc_strings_size;
	
	if (enable_buf_realocation_)
		buf_ref_.resize(new_buf_size);
	
	/** Add header name */
	for (std::size_t it = prev_buf_size, it_ = 0; 
		it < new_buf_size && it_ < name.size() ; 
		++it, ++it_) 
	{
		buf_ref_.at(it) = name.at(it_);
	}
	
	/** Add header - value separator*/
	prev_buf_size = prev_buf_size + name.size();
	for (std::size_t it = prev_buf_size, it_ = 0; 
		it < new_buf_size && it_ < misc_strings_size ; 
		++it, ++it_) 
	{
		buf_ref_.at(it) = misc_strings::name_value_separator.at(it_);
	}

	/** Add value */
	prev_buf_size = prev_buf_size + misc_strings_size;
	for (std::size_t it = prev_buf_size, it_ = 0; 
		it < new_buf_size && it_ < value.size(); 
		++it, ++it_) 
	{
		buf_ref_.at(it) = value.at(it_);
	}
}

} // namesapce utility

