#ifndef HTTP_TRANSPORT_CONTEXT_HPP_INCLUDED
#define HTTP_TRANSPORT_CONTEXT_HPP_INCLUDED

#include "http_utility.hpp"
#include "transport_context.hpp"
#include "base_transport_ostream.hpp"

#include <vector>
#include <boost/cstdint.hpp>

namespace common { 

/**
 * 
 */
class http_transport_event_handler : public transport_context {
public :
	typedef std::vector<char> buffer_type;
	
	/* context type */
	static const int context_type = __LINE__;

	/* */
	http_transport_event_handler() : transport_context(context_type) { }
	virtual ~http_transport_event_handler() { }
	
	/**
	 *
	 */

	/* Over HTTP headers operations */
	virtual void on_partial_content_request(
		base_transport_ostream_ptr ostream, std::string const & uri, utility::range_header const & range) = 0;
	virtual void on_head_request(base_transport_ostream_ptr ostream, std::string const & uri) = 0;
	virtual void on_content_request(base_transport_ostream_ptr ostream, std::string const & uri) = 0;	

};

typedef boost::shared_ptr<http_transport_event_handler> http_transport_event_handler_ptr;

} // common

#endif

