#ifndef SEND_CONTENT_REPLY_HPP_INCLUDED
#define SEND_CONTENT_REPLY_HPP_INCLUDED

#include "http_core_reply.hpp"

namespace t2h_core { namespace details {

/**
 * send_content_reply 
 * Create The 200 http reply headers.
 */
class send_content_reply : public http_core_reply {
public :
	send_content_reply();
	~send_content_reply();
	
	virtual boost::tuple<std::string, bool> get_reply_headers(http_data & hd);

};

} } // namespace t2h_core, details

#endif

