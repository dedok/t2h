#ifndef TORRENT_CORE_FUTURE_HPP_INCLUDED
#define TORRENT_CORE_FUTURE_HPP_INCLUDED

#if defined(__GNUG__)
#	pragma GCC system_header
#endif

#include <libtorrent/torrent_handle.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace t2h_core { namespace details {

class torrent_core_future 
	: public boost::enable_shared_from_this<torrent_core_future> 
{
public :
	typedef boost::shared_ptr<torrent_core_future> ptr_type;
	explicit torrent_core_future(int future_type);
	virtual ~torrent_core_future() { }

	void change_status(bool status);
	void wait_result();
	bool timed_wait_result(boost::system_time const & target_time);
	int get_type() const;	

private :
	bool volatile mutable status_;
	boost::mutex mutable lock_;	
	boost::condition_variable waiters_;
	int future_type_;

};

template <class T>
class scoped_future_promise_init : boost::noncopyable {
public :
	inline explicit scoped_future_promise_init(torrent_core_future::ptr_type & base_future) 
		: base_future_(base_future) 
	{ base_future_.reset(new T()); }
	
	~scoped_future_promise_init() 
		{ base_future_.reset();	}

private :
	torrent_core_future::ptr_type & base_future_;
}; 

typedef torrent_core_future::ptr_type torrent_core_future_ptr;

class scoped_future_release : boost::noncopyable {
public :
	explicit scoped_future_release(torrent_core_future_ptr future_ptr);
	~scoped_future_release();

private :
	torrent_core_future_ptr future_;
};


template <class T, class U>
inline boost::shared_ptr<T> future_cast(boost::shared_ptr<U> & u) 
{
	return boost::static_pointer_cast<T>(u);
}

struct add_torrent_future : public torrent_core_future {
	static int const type = __LINE__;
	add_torrent_future() : torrent_core_future(type), handle(), state(false) { }
	libtorrent::torrent_handle handle;
	bool state;
};

typedef boost::shared_ptr<add_torrent_future> add_torrent_future_ptr;

} } // namespace t2h_core, details

#endif

