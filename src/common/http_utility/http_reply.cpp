#include "http_reply.hpp"
#include "http_reply_resources.hpp"
#include "misc_utility.hpp"

#include <boost/bind.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

/**
 * Private hidden http_reply helpers
 */

namespace utility {

/**
 * Public http_reply api
 */
http_reply::http_reply(http_reply::buffer_type & buffer) 
	: status_(service_unavailable), headers_(), buf_ref_(buffer), file_info_(), no_auto_generate_headers_(false)
{
	file_info_.file_path = "";
	file_info_.from = file_info_.to = file_info_.size_for_reading = 0;
	buf_ref_.clear();
} 

http_reply::~http_reply() 
{

}

void http_reply::add_header(std::string const & name, std::string const & value) 
{
	http_header header = { name , value };
	headers_.push_back(header);
}

void http_reply::add_header(http_header const & header) 
{
	headers_.push_back(header);
}

bool http_reply::add_content_from_file(
	boost::filesystem::path const & file_path, 
	boost::int64_t from, 
	boost::int64_t to) 
{
	if (!file_path.empty()) {	
		file_info_.file_size = boost::filesystem::file_size(file_path);
		file_info_.file_path = file_path;
		file_info_.from = from; file_info_.to = to;
		if (to >= 0 && to > from)
			file_info_.size_for_reading = 1 * (to - from + 1);
		else 
			file_info_.size_for_reading = file_info_.file_size;
		
		return true;
	}
	return false;
}

bool http_reply::add_content_directly(char const * content, std::size_t content_size) 
{
	if (!content || content_size == 0) 
		return false;	
	
	try {
		add_crlf_directly();
		std::size_t const prev_buf_size = buf_ref_.size();
		std::size_t new_buf_size = prev_buf_size + content_size;
		buf_ref_.resize(new_buf_size);
		for (std::size_t it = prev_buf_size, it_ = 0; 
			it < new_buf_size && it_ < content_size; 
			++it, ++it_) 
		{
			buf_ref_.at(it) = content[it_];
		}
	} catch (std::exception const &) {
		return false;	
	} 

	return true;
}	

void http_reply::set_status(http_reply::status_type status) 
{
	status_ = status;
}

http_reply::status_type http_reply::get_status() const 
{
	return status_;
}

http_reply::formating_result http_reply::format_partial_content(
	boost::filesystem::path const & req_path, boost::int64_t start, boost::int64_t end) 
{
	formating_result result = io_error;
	try 
	{
		if (!add_content_from_file(req_path, start, end))
			return unknown_error;
		
		if (!no_auto_generate_headers_) {
			std::string const range_value = 
				"bytes " + safe_lexical_cast<std::string>(start) + "-" + 
				safe_lexical_cast<std::string>(file_info_.size_for_reading - 1) + "/" + 
				safe_lexical_cast<std::string>(file_info_.file_size);

			set_status(partial_content);
			status_strings::cast_to_buffer(status_, buf_ref_);
			add_header_directly("Accept-Ranges", "bytes");
			add_header_directly("Content-Range", range_value);
			add_header_directly("Content-Length", 
				safe_lexical_cast<std::string>(file_info_.size_for_reading));
			// TODO add mime type into http header
			//add_header_directly("Content-Type", "");
		}
		
		result = fill_content_from_file();
	} 
	catch (std::exception const &)
	{
		return unknown_error;
	}
	return result;
}

void http_reply::stock_reply(http_reply::status_type status) 
{
	reset_buffer();
	set_status(status);

	std::string const content = stock_replies::cast_to_string(status);
	std::size_t const content_size = content.size();
	if (!no_auto_generate_headers_) {
		status_strings::cast_to_buffer(status, buf_ref_);
		add_header_directly("Content-Length", safe_lexical_cast<std::string>(content_size));
		add_header_directly("Content-Type", "text/html");
	}
	add_content_directly(content.c_str(), content_size);
} 

void http_reply::reset_buffer() 
{
	status_ = service_unavailable;
	buf_ref_.clear();
	add_content_from_file("", 0, 0);
}

void http_reply::set_no_auto_generate_headers(bool state) const 
{
	no_auto_generate_headers_ = state;
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

	/** Add end */	
	add_crlf_directly();
}

void http_reply::add_crlf_directly() 
{
	std::copy(misc_strings::crlf.begin(), misc_strings::crlf.end(), 
		std::back_inserter(buf_ref_));
}

http_reply::formating_result http_reply::fill_content_from_file() 
{
	//TODO fix types compability
	namespace io = boost::iostreams;
	formating_result result = file_not_exist; 
	char * buffer_ptr = NULL;
	std::ios::openmode const open_mode = std::ios::in | std::ios::binary;
	io::file_descriptor_source file_handle(file_info_.file_path.string(), open_mode);
	if (file_handle.is_open()) {
		if (io::seek(file_handle, file_info_.from, BOOST_IOS::beg) >= 0) {
			std::size_t const prev_buff_size = buf_ref_.size();
			buf_ref_.resize(prev_buff_size + file_info_.size_for_reading);
			buffer_ptr = &buf_ref_.at(prev_buff_size);
			(io::read(file_handle, buffer_ptr, file_info_.size_for_reading) != -1) ?
				result = formating_succ : result = io_error;	
		} // !if
		return result;
	} // !if
	
	return result;
}

} // namespace utility
