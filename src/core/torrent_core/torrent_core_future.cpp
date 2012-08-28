#include "torrent_core_future.hpp"

namespace t2h_core { namespace details {

torrent_core_future::torrent_core_future(int future_type) 
	: status_(false), lock_(), future_type_(future_type) { }

void torrent_core_future::change_status(bool status) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	status_ = status;
}

void torrent_core_future::wait_result() const 
{
	// TODO impl follow logic via condition variables + add timed wait
	for (;;) {
		boost::lock_guard<boost::mutex> guard(lock_);
		if (status_) break;
	}
}

int torrent_core_future::get_type() const 
{
	return future_type_;
}

scoped_future_release::scoped_future_release(torrent_core_future_ptr future_ptr) 
	: future_(future_ptr) { }

scoped_future_release::~scoped_future_release() 
{
	if (future_)
		future_->change_status(true);
}

} }

