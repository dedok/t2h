#include "file_info_buffer.hpp"

#include "http_server_macroses.hpp"
#include "core_notification_center.hpp"
#include "file_info_buffer_realtime_updater.hpp"

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define HCORE_FIB_UPDATER_NAME "hcore_notification_recv";

namespace t2h_core { namespace details {

/**
 * Public api
 */

file_info_buffer_ptr shared_file_info_buffer() 
{
	static file_info_buffer_ptr fibp(new file_info_buffer());
	return fibp;
}

/**
 * Public file_info_buffer api
 */

file_info_buffer::file_info_buffer() 
	: lock_(), infos_(), updater_()
{
	updater_.recv_name = HCORE_FIB_UPDATER_NAME;
	updater_.nr.reset(new file_info_buffer_realtime_updater(*this, updater_.recv_name));
	core_notification_center()->add_notification_receiver(updater_.nr);
}

file_info_buffer::~file_info_buffer() 
{
	core_notification_center()->remove_notification_receiver(updater_.recv_name);
	boost::mutex::scoped_lock guard(lock_);
	guard.unlock();
	waiter_.notify_one();
}

bool file_info_buffer::wait_avaliable_bytes(
		std::string const & file_path, boost::int64_t avaliable_bytes, std::size_t secs) 
{
	/* */
	using boost::posix_time::seconds;
	
	hc_file_info_ptr finfo;
	
	if ((finfo = get_info(file_path))) {
		boost::mutex::scoped_lock guard(lock_);
		for (;;) {	
			if (finfo->avaliable_bytes >= avaliable_bytes) {
#if defined(T2H_DEEP_DEBUG)
				HCORE_TRACE("wait bytes for path '%s' finished", finfo->file_path.c_str())
#endif // T2H_DEEP_DEBUG
				return true;
			}
			boost::system_time timeout = boost::get_system_time() + seconds(secs);
			if (!waiter_.timed_wait(guard, timeout)) {
				HCORE_WARNING("wait bytes for path '%s' failed", finfo->file_path.c_str())
				return false;
			}
		} // wait loop
	} // state
	HCORE_WARNING("finfo not valid for path '%s'", file_path.c_str())
	return false;
}

void file_info_buffer::remove_info(std::string const & path) 
{
	boost::mutex::scoped_lock guard(lock_);
	infos_type::iterator found = infos_.find(path);
	if (found != infos_.end()) {
#if defined(T2H_DEEP_DEBUG)
		HCORE_TRACE("removing file info entry '%s'", found->second->file_path.c_str())
#endif // T2H_DEEP_DEBUG
		infos_.erase(found);	
	}
}

void file_info_buffer::update_info(hc_file_info_ptr info) 
{
	BOOST_ASSERT(info != NULL);
	boost::mutex::scoped_lock guard(lock_);
	
	infos_type::iterator found = infos_.find(info->file_path);
	if (found == infos_.end()) {
		HCORE_WARNING("update info failed, item not found", info->file_path.c_str())
		return;
	}
#if defined(T2H_DEEP_DEBUG)
	HCORE_TRACE("updating existing file info entry '%s', bytes avaliable '"SL_SSIZE_T"'", 
		info->file_path.c_str(), info->avaliable_bytes)
#endif // T2H_DEEP_DEBUG
	found->second->file_size = info->file_size;
	found->second->avaliable_bytes = info->avaliable_bytes;
	guard.unlock();
	waiter_.notify_one();
}

void file_info_buffer::update_info(std::string const & file_path, boost::int64_t avaliable_bytes) 
{
	boost::mutex::scoped_lock guard(lock_);
	
	infos_type::iterator found = infos_.find(file_path);
	if (found == infos_.end()) {
		HCORE_WARNING("update info failed, item not found", file_path.c_str())
		return;
	}
#if defined(T2H_DEEP_DEBUG)
	HCORE_TRACE("updating existing item '%s', bytes avaliable '"SL_SSIZE_T"'", 
		found->second->file_path.c_str(), avaliable_bytes)
#endif // T2H_DEEP_DEBUG
	found->second->avaliable_bytes = avaliable_bytes;
	guard.unlock();
	waiter_.notify_one();
}
	
hc_file_info_ptr file_info_buffer::get_info(std::string const & path) const 
{
	boost::mutex::scoped_lock guard(lock_);
	infos_type::const_iterator found = infos_.find(path);
	if (found != infos_.end())
		return found->second;
	return hc_file_info_ptr();
}

/**
 * Private file_info_buffer api
 */

} } // namespace t2h_core, details

#undef HCORE_FIB_UPDATER_NAME
