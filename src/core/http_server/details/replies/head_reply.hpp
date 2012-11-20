#ifndef HEAD_REPLY_HPP_INCLUDED
#define HEAD_REPLY_HPP_INCLUDED

#include "http_core_reply.hpp"

namespace t2h_core { namespace details {

/**
 * head_reply
 */
class head_reply : public http_core_reply {
public :
	head_reply();
	~head_reply();
	
	virtual boost::tuple<std::string, bool> get_reply_headers(http_data & hd);

}; 

} } // namespace t2h_core, details

#endif

