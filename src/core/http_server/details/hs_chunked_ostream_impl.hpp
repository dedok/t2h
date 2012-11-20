#ifndef HS_CHUNKED_OSTREAM_IMPL_HPP_INCLUDED
#define HS_CHUNKED_OSTREAM_IMPL_HPP_INCLUDED

#include "http_server_ostream_policy.hpp"

namespace t2h_core { namespace details {

/**
 * http_chunked_ostream chunked outout stream for http reply 
 */

struct hs_chunked_ostream_params {
	boost::int64_t max_chunk_size;
	std::size_t cores_sync_timeout;
};

class hs_chunked_ostream_impl : public http_server_ostream_policy {
public :
	hs_chunked_ostream_impl(http_server_ostream_policy_params const & base_params, hs_chunked_ostream_params const & params);
	~hs_chunked_ostream_impl();
	
protected :
	virtual bool write_content_impl(http_data & hd);

private :
	hs_chunked_ostream_params mutable params_;

};

} } // namespace t2h_core, details

#endif

