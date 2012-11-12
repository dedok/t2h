#ifndef FILE_INFO_BUFFER_REALTIME_UPDATER_HPP_INCLUDED
#define FILE_INFO_BUFFER_REALTIME_UPDATER_HPP_INCLUDED

#include "file_info_buffer.hpp"
#include "notification_receiver.hpp"

namespace t2h_core { namespace details { 

/**
 * file_info_buffer_realtime_updater, event receiver needed to update/add/remove/insert file_info_buffer items 
 */
struct file_info_buffer_realtime_updater : public common::notification_receiver {
	file_info_buffer_realtime_updater(file_info_buffer & fib, std::string const & recv_name);
	~file_info_buffer_realtime_updater();
	
	virtual void on_notify(common::notification_ptr notification);	
	virtual void on_notify_failed(common::notification_ptr notification, int reason);

	file_info_buffer & fib_;

}; 

} } // namespace t2h_core, details

#endif

