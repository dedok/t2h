#ifndef ROOT_REPLY_HPP_INCLUDED
#define ROOT_REPLY_HPP_INCLUDED

#include "http_reply.hpp"

namespace t2h_core { namespace details {

/**
 * root_reply_param
 */
struct root_reply_param {
};

/**
 * root_reply
 */
class root_reply : public utility::http_reply {
public :
	root_reply(utility::http_reply::buffer_type & buf_ref, root_reply_param const & param);
	~root_reply();
	
	virtual bool do_formatting_reply();

private :
	root_reply_param mutable param_;

}; 

} } // namespace t2h_core, details

#endif

