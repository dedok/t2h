#ifndef HTTP_SERVER_OSTREAM_POLICY_HPP_INCLUDED
#define HTTP_SERVER_OSTREAM_POLICY_HPP_INCLUDED

#include "http_core_reply.hpp"
#include "base_transport_ostream.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core { namespace details {

/**
 *
 */
struct http_server_ostream_policy_params {
	bool reset_stream_at_end_of_io;
};

/**
 *
 */
class http_server_ostream_policy : private boost::noncopyable 
{
public :
	explicit http_server_ostream_policy(http_server_ostream_policy_params const & params);
	virtual ~http_server_ostream_policy();
	
	inline void set_ostream(common::base_transport_ostream_ptr ostream_impl) 
		{ ostream_impl_ = ostream_impl; }

	inline common::base_transport_ostream_ptr get_ostream() 
		{ return ostream_impl_; }

	bool perform(http_core_reply & reply, http_data & hd);

protected :
	virtual bool write_content_impl(http_data & hd) = 0;
	common::base_transport_ostream_ptr ostream_impl_;

private :
	http_server_ostream_policy_params mutable base_params_;

};

typedef boost::shared_ptr<http_server_ostream_policy> http_server_ostream_policy_ptr;

} } // namespace t2h_core, details


#endif

