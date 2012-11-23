#ifndef HS_CHUNKED_OSTREAM_IMPL_HPP_INCLUDED
#define HS_CHUNKED_OSTREAM_IMPL_HPP_INCLUDED

#include "base_chunked_ostream.hpp"

#include <boost/thread.hpp>

namespace t2h_core { namespace details {

/**
 *
 */
class hs_chunked_ostream_impl : public base_chunked_ostream {
public :
	enum { is_breaked = 1, state_default = 2 };

	hs_chunked_ostream_impl(http_server_ostream_policy_params const & base_params, hs_chunked_ostream_params const & params);
	~hs_chunked_ostream_impl();

	/* From async_file_info_subscriber */
	virtual void on_bytes_avaliable_change(boost::int64_t avaliable_bytes);
	virtual void on_break();
	virtual async_file_info_subscriber * clone() 
		{ return NULL; }

protected :
	virtual bool write_content_impl(http_data & hd);

private :
	bool wait_for_bytes(boost::int64_t bytes);

	hs_chunked_ostream_params mutable params_;
	
	struct {
		boost::mutex waiter_lock;				// 
		boost::condition_variable waiter;		//
		boost::int64_t avaliable_bytes;			//
		int state;								//
	} ex_data_;

};

} } // namespace t2h_core, details

#endif

