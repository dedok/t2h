#ifndef TORRENT_CORE_FUTURE_HPP_INCLUDED
#define TORRENT_CORE_FUTURE_HPP_INCLUDED

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
	explicit torrent_core_future(int future_type);
	virtual ~torrent_core_future() { }

	void change_status(bool status);
	void wait_result() const;
	int get_type() const;	

private :
	bool volatile status_;
	boost::mutex mutable lock_;
	int future_type_;

};

typedef boost::shared_ptr<torrent_core_future> torrent_core_future_ptr;

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

