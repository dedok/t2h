#ifndef BASE_CHUNKED_OSTREAM_HPP_INCLUDED
#define BASE_CHUNKED_OSTREAM_HPP_INCLUDED

#include "http_server_ostream_policy.hpp"
#include "async_file_info_subscriber.hpp"

namespace t2h_core { namespace details {

/**
 * http_chunked_ostream chunked outout stream for http reply 
 */
struct hs_chunked_ostream_params {
	boost::int64_t max_chunk_size;
	std::size_t cores_sync_timeout;
};

class base_chunked_ostream : 
	public http_server_ostream_policy, 
	public async_file_info_subscriber 
{
public :
	base_chunked_ostream(
		http_server_ostream_policy_params const & base_params, hs_chunked_ostream_params const & params) 
		: http_server_ostream_policy(base_params), async_file_info_subscriber() { }
	
	virtual ~base_chunked_ostream() { }

};

typedef boost::shared_ptr<base_chunked_ostream> chunked_ostream_ptr;


} } // namespace t2h_core, details

#endif

