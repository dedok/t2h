#include "file_info_buffer.hpp"

#include "http_server_macroses.hpp"
#include "core_notification_center.hpp"
#include "file_info_buffer_realtime_updater.hpp"

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//#define T2H_DEEP_DEBUG
#define HCORE_FIB_UPDATER_NAME "hcore_notification_recv";

namespace t2h_core { namespace details {

/**
 * Public file_info_buffer api
 */

file_info_buffer_ptr shared_file_info_buffer() 
{
	static file_info_buffer_ptr fibp(new file_info_buffer());
	return fibp;
}


file_info_buffer::file_info_buffer() 
	: is_stoped_(false), lock_(), infos_(), updater_()
{
	updater_.recv_name = HCORE_FIB_UPDATER_NAME;
	updater_.nr.reset(new file_info_buffer_realtime_updater(*this, updater_.recv_name));
	core_notification_center()->add_notification_receiver(updater_.nr);
}

file_info_buffer::~file_info_buffer() 
{
	stop_force();
}

int file_info_buffer::registr_subscriber(hc_file_info_ptr fi, async_file_info_subscriber_ptr subscriber) 
{
	BOOST_ASSERT(fi != NULL);
	// Registr new subscriber and send notification about bytes avaliable
	boost::lock_guard<boost::mutex> guard(lock_);
	fi->subscribers.push_back(subscriber);
	subscriber->on_bytes_avaliable_change(fi->avaliable_bytes);
	return fi->subscribers.size();
}

void file_info_buffer::unregistr_subscriber(hc_file_info_ptr fi, int sid) 
{
	BOOST_ASSERT(fi != NULL);
	boost::lock_guard<boost::mutex> guard(lock_);
	if (sid > fi->subscribers.size())
		fi->subscribers.erase(fi->subscribers.begin() + sid);
}

void file_info_buffer::unregistr_subscriber(hc_file_info_ptr fi, async_file_info_subscriber_ptr subscriber) 
{
	BOOST_ASSERT(fi != NULL);
	boost::lock_guard<boost::mutex> guard(lock_);
	std::vector<async_file_info_subscriber_ptr>::iterator first = fi->subscribers.begin(), 
		last = fi->subscribers.end();
	for (;
		first != last; 
		++first) 
	{
		if ((*first) == subscriber) {
			(*first)->on_break();
			fi->subscribers.erase(first);
			break;
		}
	} // for
}

void file_info_buffer::remove_info(std::string const & path) 
{
	boost::lock_guard<boost::mutex> guard(lock_);

	if (is_stoped_)
		return;

	infos_type::iterator found = infos_.find(path);
	if (found != infos_.end()) 
		infos_.erase(found); 
	
	std::for_each(found->second->subscribers.begin(), found->second->subscribers.end(), 
		boost::bind(&async_file_info_subscriber::on_break, _1));
}

void file_info_buffer::update_info(std::string const & file_path, boost::int64_t avaliable_bytes) 
{
	boost::mutex::scoped_lock guard(lock_);
	
	if (is_stoped_)
		return;

	infos_type::iterator found = infos_.find(file_path);
	if (found == infos_.end()) {
		HCORE_WARNING("update info failed, item not found", file_path.c_str())
		return;
	}
	found->second->avaliable_bytes = avaliable_bytes;
	
	std::for_each(found->second->subscribers.begin(), found->second->subscribers.end(), 
		boost::bind(&async_file_info_subscriber::on_bytes_avaliable_change, _1, avaliable_bytes));
}
	
hc_file_info_ptr file_info_buffer::get_info(std::string const & path) const 
{
	boost::mutex::scoped_lock guard(lock_);
	
	if (is_stoped_) 
		return hc_file_info_ptr();

	infos_type::const_iterator found = infos_.find(path);
	return (found != infos_.end()) ? found->second : hc_file_info_ptr();
}

/**
 * Private file_info_buffer api
 */

void file_info_buffer::stop(bool graceful) 
{
	/*  If still somebody waits a bites we must notify about buffer does not work,
	 	also remove self updater from notification center and clear all data in buffer */
	boost::mutex::scoped_lock guard(lock_);
	core_notification_center()->remove_notification_receiver(updater_.recv_name);
	is_stoped_ = true;
	for (infos_type::iterator first = infos_.begin(), last = infos_.end(); 
		first != last; 
		++first) 
	{
		hc_file_info_notify_subscribers(first->second, 
			boost::bind(&async_file_info_subscriber::on_break, _1));
	}
	infos_.clear();
	is_stoped_ = false;
}

} } // namespace t2h_core, details

#undef HCORE_FIB_UPDATER_NAME
