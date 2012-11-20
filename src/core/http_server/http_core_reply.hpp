#ifndef HTTP_CORE_REPLY_HPP_INCLUDED
#define HTTP_CORE_REPLY_HPP_INCLUDED

#include "file_info_buffer.hpp"

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>

namespace t2h_core { namespace details {

/**
 * http_data store all needed data for send reply to client
 */
struct http_data {
	hc_file_info_ptr fi;						// Pointer to file information
	file_info_buffer_ptr fi_buffer;				// Pointer to file information buffer

	boost::int64_t read_start;					// Read start offset
	boost::int64_t read_end;					// Read end offset
};

/**
 * base_http_core_reply base class for all reply types
 * To send reply via abstract transport stream just call base_http_core_reply::perform
 */
class http_core_reply : boost::noncopyable {
public :
	http_core_reply() : boost::noncopyable() { }
	virtual ~http_core_reply() { }
	/* std::string containt headers, bool means is need send content from file or not */
	virtual boost::tuple<std::string, bool> get_reply_headers(http_data & hd) = 0;

private :

};

} } // namespace t2h_core, details

#endif 

