#include "partial_content_reply.hpp"
#include "misc_utility.hpp"
#include "mime_types.hpp"

#include <boost/iostreams/operations.hpp>  
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/file_descriptor.hpp> 

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

namespace t2h_core { namespace details {

/**
 *
 */
static inline void get_date_time(std::string & out) 
{
	// TODO impl this
}

/**
 * Public partial_content_reply
 */

partial_content_reply::partial_content_reply(
	utility::http_reply::buffer_type & buf_ref, partial_content_reply_param const & param) 
	: utility::http_reply(buf_ref), 
	param_(param) 
{ 
}

partial_content_reply::~partial_content_reply() 
{

}

bool partial_content_reply::do_formatting_reply() 
{
	using namespace utility;
	
	if (!ready_for_reply())
		return false;
	
	std::string const range_value = 
		"bytes " + safe_lexical_cast<std::string>(param_.bytes_start) + "-" + 
		safe_lexical_cast<std::string>(param_.bytes_end) + "/" + 
		safe_lexical_cast<std::string>(param_.file_size);
		
	// TODO add Date as header
	if (!http_reply::add_status(http_reply::partial_content))
		return false;

	//std::string date_time;
	//get_date_time(date_time);
	//if (!http_reply::add_status("Date", date_time)
	//	return false;
	
	if (!http_reply::add_header("Accept-Ranges", "bytes"))
		return false;

	if (!http_reply::add_header("Content-Range", range_value))
		return false;
	
	if (!http_reply::add_header("Content-Length", safe_lexical_cast<std::string>(param_.size_for_reading)))
		return false;

	if (!http_reply::add_header("Content-Type", mime_types::get_file_extension(param_.file_path)))
		return false;
	
	return fill_content_from_file();
}

/**
 * Private partial_content_reply api
 */

bool partial_content_reply::ready_for_reply() 
{
	if (param_.bytes_end >= param_.file_size) 
		param_.bytes_end = param_.file_size;
	param_.size_for_reading = (1 * (param_.bytes_end - param_.bytes_start)) + 1;
	return ((param_.size_for_reading == 0) ? false : true);
}

bool partial_content_reply::fill_content_from_file() 
{
	using namespace utility;
	namespace io = boost::iostreams;
	try 
	{
		http_reply::buffer_type & buf_ref = http_reply::get_buffer(); 
		std::ios::openmode const open_mode = std::ios::in | std::ios::binary;
		io::file_descriptor_source file_handle(param_.file_path.string(), open_mode);
		if (file_handle.is_open()) {
			http_reply::add_crlf();
			if (io::seek(file_handle, param_.bytes_start, BOOST_IOS::beg) >= 0) {
				std::size_t const prev_buff_size = buf_ref.size();
				if (enable_buf_realocation_)
					buf_ref.resize(prev_buff_size + param_.size_for_reading);
				char * buffer_offset_start = &buf_ref.at(prev_buff_size);
				if (io::read(file_handle, buffer_offset_start, param_.size_for_reading) > 0) 
					return true; 
			} // !if
		} // !if
	}
	catch (std::exception const &) { }
	return false;
}

} } // namesapce t2h_core, details

