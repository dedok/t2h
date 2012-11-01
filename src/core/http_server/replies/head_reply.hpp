#ifndef HEAD_REPLY_HPP_INCLUDED
#define HEAD_REPLY_HPP_INCLUDED

#include "http_reply.hpp"

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>

namespace t2h_core { namespace details {

/**
 * head_reply_param
 */
struct head_reply_param {
	boost::filesystem::path file_path;			// path to file with doc_root 
	boost::int64_t expected_file_size;			// expected(real) file size
};

/**
 * head_reply
 */
class head_reply : public utility::http_reply {
public :
	head_reply(utility::http_reply::buffer_type & buf_ref, head_reply_param const & param);
	~head_reply();
	
	virtual bool do_formatting_reply();

private :
	head_reply_param mutable param_;

}; 

} } // namespace t2h_core, details

#endif

