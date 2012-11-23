#ifndef ASYNC_FILE_INFO_SUBSCRIBER_HPP_INCLUDED
#define ASYNC_FILE_INFO_SUBSCRIBER_HPP_INCLUDED

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core { namespace details {

/**
 *
 */

class async_file_info_subscriber : boost::noncopyable {
public :
	async_file_info_subscriber() { }
	virtual ~async_file_info_subscriber() { }
	
	virtual void on_bytes_avaliable_change(boost::int64_t avaliable_bytes) = 0;
	virtual void on_break() = 0;
	virtual async_file_info_subscriber * clone() = 0;
	
	inline intptr_t get_address() const 
		{ return (intptr_t)this; }

};

/**
 *
 */

typedef boost::shared_ptr<async_file_info_subscriber> async_file_info_subscriber_ptr;

inline bool operator==(async_file_info_subscriber const & rhs, async_file_info_subscriber const & lhs) 
	{ return (rhs.get_address() == lhs.get_address()); }

inline bool operator==(async_file_info_subscriber_ptr rhs, async_file_info_subscriber_ptr lhs) 
	{ return operator==(*rhs.get(), *lhs.get()); }

} } // namespace t2h_core, details

#endif

