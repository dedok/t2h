#include "hs_chunked_ostream_impl.hpp"

#include "http_server_macroses.hpp"

#include <boost/iostreams/operations.hpp>  
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/file_descriptor.hpp> 

#define T2H_DEEP_DEBUG

namespace t2h_core { namespace details {

/**
 * Public hs_chunked_ostream_impl api
 */

hs_chunked_ostream_impl::hs_chunked_ostream_impl(
	http_server_ostream_policy_params const & base_params, hs_chunked_ostream_params const & params) 
	: http_server_ostream_policy(base_params), params_(params) 
{
	
}

hs_chunked_ostream_impl::~hs_chunked_ostream_impl() 
{

}

/**
 * Protected hs_chunked_ostream_impl api
 */

bool hs_chunked_ostream_impl::write_content_impl(http_data & hd) 
{
	/*  First we must to ensure about we have needed bytes in file,
	 	because real file size could be less then bytes_end(at moment of call). 
		if file size > than request/chunk then we 
		sync with torrent_core via blocking call 'wait_avaliable_bytes'.
		If bytes are avaliable then just send block to clien otherwise error. */
	namespace io = boost::iostreams;
		
	bool eof = false;
	std::vector<char> obuffer(params_.max_chunk_size + 10);
	std::ios::openmode const open_mode = std::ios::in | std::ios::binary;
	for (boost::int64_t read_offset = params_.max_chunk_size, seek_pos = hd.read_start, 
			readed = 0, bwait = 0;
		;
		) 
	{
		if ((seek_pos + read_offset) > hd.read_end) {
			read_offset = 1 * ((hd.read_end - seek_pos)) + 1;
			bwait = hd.read_end + 1 > hd.fi->file_size ? hd.fi->file_size : hd.read_end + 1;
			eof = true;
		} else {
			if ((seek_pos + read_offset) == hd.read_end)
				eof = true;
			bwait = seek_pos + read_offset;
		}
		
		if (!hd.fi_buffer->wait_avaliable_bytes(hd.fi, bwait, params_.cores_sync_timeout)) {
			HCORE_WARNING("waiting fot bytes failed, '%s'", hd.fi->file_path.c_str())
			return false;
		} // if	
		
		std::cout << "read_offset : " << read_offset 
			<< " seek_pos : " << seek_pos << " readed : " << readed << " bwait : " << bwait
			<< std::endl;
		
		{ // start file io scope	
		io::file_descriptor_source file_handle(hd.fi->file_path, open_mode);
		if (!file_handle.is_open()) { 
			HCORE_WARNING("failed to open file '%s'", hd.fi->file_path.c_str())
			return false;
		} // if
	
		if (io::seek(file_handle, seek_pos, BOOST_IOS::beg) < 0) { 
			HCORE_WARNING("failed to seek to pos '%i' file '%s'", seek_pos, hd.fi->file_path.c_str())
			return false;
		} // if
		
		if ((readed =
			io::read(file_handle, &obuffer.at(0), read_offset)) <= 0) 
		{ 
			HCORE_WARNING("failed to read file '%s' from '%i' to '%i'", hd.fi->file_path.c_str(), seek_pos, read_offset)
			return false;
		} // if
		} // end file io scope
		
		if (ostream_impl_->write(&obuffer.at(0), readed) > readed) {
			HCORE_WARNING("failed to write data for '%s'", hd.fi->file_path.c_str())
			return false;
		}
#if defined(T2H_DEEP_DEBUG)
		HCORE_TRACE("with file '%s', writed to ostream '%i', seek pos '%i', read_offset '%i', read end '%i', wait '%i'", 
			hd.fi->file_path.c_str(), readed, seek_pos, read_offset, hd.read_end, bwait)
#endif // T2H_DEEP_DEBUG
		if (eof)  
			return true;
	
		seek_pos = seek_pos + readed;	
	} // for

	return false;
} 

/**
 * Private hs_chunked_ostream_impl api
 */

} } // namespace t2h_core, details

