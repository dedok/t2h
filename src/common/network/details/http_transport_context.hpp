#ifndef HTTP_TRANSPORT_CONTEXT_HPP_INCLUDED
#define HTTP_TRANSPORT_CONTEXT_HPP_INCLUDED

#include "transport_context.hpp"

#include <vector>
#include <boost/cstdint.hpp>

namespace common { 

/**
 *
 */
class http_transport_event_handler : public transport_context {
public :
	typedef std::vector<char> buffer_type;
	
	/**
	 *
	 */
 	enum operation_status {
		ok = 0x0,
		io_error = 0x2,	
		not_found = 0x1,
		bad_request = 0x3,
		write_op_error = 0x4,
		unknown = write_op_error + 0x1
	};

	/**
	 *
	 */
	struct http_data { 
		buffer_type io_buffer;						//	
		std::string reply_header;					//
		operation_status op_status;					//
		boost::int64_t last_readed;					//
		boost::int64_t seek_offset_pos;				//
	};
	
	static const int context_type = __LINE__;
	http_transport_event_handler() : transport_context(context_type) { }
	virtual ~http_transport_event_handler() { }

	/* Over HTTP headers operations */
	virtual void on_get_partial_content_headers(
		http_data & http_d, boost::int64_t bytes_start, boost::int64_t bytes_end, char const * uri) = 0;	
	virtual void on_get_content_headers(http_data & http_d, char const * uri) = 0;
	virtual void on_get_head_headers(http_data & http_d, char const * uri) = 0;
	
	/* Operations with content data */
	virtual bool on_get_content_body(
			http_data & http_d, 
			boost::int64_t bytes_start, 
			boost::int64_t bytes_end, 
			boost::int64_t bytes_writed,
			char const * uri) = 0;	
	
	/* Informers/Heplers */
	virtual void error(operation_status status, char const * uri) = 0;

};

typedef boost::shared_ptr<http_transport_event_handler> http_transport_event_handler_ptr;

} // common

#endif

