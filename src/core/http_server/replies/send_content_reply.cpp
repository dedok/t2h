#include "send_content_reply.hpp"

#include "misc_utility.hpp"
#include "mime_types.hpp"

#include <boost/iostreams/operations.hpp>  
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/file_descriptor.hpp> 

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

namespace t2h_core { namespace details {

/**
 * Public send_content_reply api
 */

send_content_reply::send_content_reply(
	utility::http_reply::buffer_type & buf_ref, send_content_reply_param const & param) 
	: utility::http_reply(buf_ref), param_(param) { }

send_content_reply::~send_content_reply() 
	{ }

bool send_content_reply::do_formatting_reply() 
{
	using namespace utility;
	
	if (!add_status(http_reply::ok))
		return false;

	if (!http_reply::add_header("Content-Length", safe_lexical_cast<std::string>(param_.file_size)))
		return false;

	if (!http_reply::add_header("Content-Type", mime_types::get_file_extension(param_.file_path)))
		return false;

	return fill_content_from_file();
}

/**
 * Private send_content_reply api
 */

bool send_content_reply::fill_content_from_file() 
{
	using namespace utility;
	namespace io = boost::iostreams;
	try 
	{
		http_reply::buffer_type & buf_ref = http_reply::get_buffer(); 
		std::ios::openmode const open_mode = std::ios::in | std::ios::binary;
		io::file_descriptor_source file_handle(param_.file_path, open_mode);
		if (file_handle.is_open()) {
			http_reply::add_crlf();
			if (io::seek(file_handle, 0, BOOST_IOS::beg) >= 0) {
				std::size_t const prev_buff_size = buf_ref.size();
				if (enable_buf_realocation_)
					buf_ref.resize(prev_buff_size + param_.file_size);
				char * buffer_offset_start = &buf_ref.at(prev_buff_size);
				if (io::read(file_handle, buffer_offset_start, param_.file_size) > 0) 
					return true; 
			} // !if
		} // !if
	}
	catch (std::exception const &) { }
	return false;
}


} } // namespace t2h_core, details

