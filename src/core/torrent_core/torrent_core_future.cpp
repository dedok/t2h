#include "torrent_core_future.hpp"

namespace t2h_core { namespace details {

torrent_core_future::torrent_core_future(int future_type) 
	: status_(false), lock_(), waiters_(), future_type_(future_type) { }

void torrent_core_future::change_status(bool status) 
{
	status_ = status;
	waiters_.notify_all();
}

void torrent_core_future::wait_result() 
{
	boost::unique_lock<boost::mutex> guard(lock_);
	for (;;) {
		waiters_.wait(guard);
		if (status_) break;
	}
}

bool torrent_core_future::timed_wait_result(boost::system_time const & target_time) 
{
	boost::unique_lock<boost::mutex> guard(lock_);
	for(;;) {
		bool const success = waiters_.timed_wait(guard, target_time);
		if (!status_ && !success) return false;
		if (status_) break;
	}
	return true;
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

} } // namespace t2h_core, details

