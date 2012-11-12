#ifndef PARTIAL_CONTENT_REPLY_HPP_INCLUDED
#define PARTIAL_CONTENT_REPLY_HPP_INCLUDED

#include "http_reply.hpp"

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>

namespace t2h_core { namespace details {

/**
 * partial_content_reply_param
 */
struct partial_content_reply_param {
	boost::filesystem::path file_path;			// [R] Path to existing file with ready for read data inside 
	boost::int64_t bytes_start;					// [R] Read start offset 
	boost::int64_t bytes_end;					// [R] Read end offset
	boost::int64_t file_size;					// [R] Size of file(file_path var)
	boost::int64_t size_for_reading;			// [R] Size for reading
	boost::int64_t content_size;				// [O]
};

static inline partial_content_reply_param create_partial_content_param(
	boost::filesystem::path const & path, 
	boost::int64_t file_size, 
	boost::int64_t bs, 
	boost::int64_t be, 
	boost::int64_t sfr,
	boost::int64_t cs = -1) 
{
	partial_content_reply_param const param = { 
		path, bs, be, file_size, sfr, cs 
	};
	return param;
}

/**
 * partial_content_reply  
 */
class partial_content_reply : public utility::http_reply {
public :
	partial_content_reply(utility::http_reply::buffer_type & buf_ref, partial_content_reply_param const & param);
	~partial_content_reply();
	
	virtual bool do_formatting_reply();

private :
	bool fill_content_from_file();
	
	partial_content_reply_param mutable param_;

};

} } // namespace t2h_core, details

#endif

