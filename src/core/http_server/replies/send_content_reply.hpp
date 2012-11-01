#ifndef SEND_CONTENT_REPLY_HPP_INCLUDED
#define SEND_CONTENT_REPLY_HPP_INCLUDED

#include "http_reply.hpp"

#include <boost/cstdint.hpp>

namespace t2h_core { namespace details {

/**
 * Parameters for send_content_reply
 */
struct send_content_reply_param {
	std::string file_path;				// full path to file system
	boost::int64_t file_size;			// file size	
};

/**
 * send_content_reply add needed headers and 
 * wrap all file(send_content_reply_param::file_path) bytes to reply buffer 
 */
class send_content_reply : public utility::http_reply {
public :
	send_content_reply(utility::http_reply::buffer_type & buf_ref, send_content_reply_param const & param);
	~send_content_reply();
	
	virtual bool do_formatting_reply();

private :
	bool fill_content_from_file();

	send_content_reply_param mutable param_;

};

} } // namespace t2h_core, details

#endif

