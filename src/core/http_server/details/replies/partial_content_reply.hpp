#ifndef PARTIAL_CONTENT_REPLY_HPP_INCLUDED
#define PARTIAL_CONTENT_REPLY_HPP_INCLUDED

#include "http_core_reply.hpp"

namespace t2h_core { namespace details {

/**
 * partial_content_reply  
 * Create 206(Partial-Content) http header.
 */	
class partial_content_reply : public http_core_reply {
public :
	partial_content_reply();
	~partial_content_reply();
	
	virtual boost::tuple<std::string, bool> get_reply_headers(http_data & hd);
};

} } // namespace t2h_core, details

#endif

