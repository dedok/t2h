#include "file_info_buffer_realtime_updater.hpp"
#include "core_file_change_notification.hpp"

namespace t2h_core { namespace details {

/**
 * Public file_info_buffer_realtime_updater api
 */
file_info_buffer_realtime_updater::file_info_buffer_realtime_updater(file_info_buffer & fib, std::string const & recv_name) 
	: common::notification_receiver(recv_name), fib_(fib) 
{ 
}

file_info_buffer_realtime_updater::~file_info_buffer_realtime_updater() 
{
}
	
void file_info_buffer_realtime_updater::on_notify(common::notification_ptr notification) 
{
	core_file_change_notification_ptr file_change_notification 
		= common::notification_cast<core_file_change_notification>(notification);
	if (file_change_notification) {
		switch (file_change_notification->event_type) {
			case core_file_change_notification::file_remove : 
				fib_.on_file_remove(file_change_notification->file_path);
			break;
			case core_file_change_notification::file_add :
				fib_.on_file_add(file_change_notification->file_path, 
					file_change_notification->file_size, file_change_notification->avaliable_bytes);
			break;
			case core_file_change_notification::file_update :
				fib_.on_file_update(file_change_notification->file_path, 
					file_change_notification->file_size, file_change_notification->avaliable_bytes);
			break;
		} // switch
		file_change_notification->set_state(common::base_notification::done);
	}
}

void file_info_buffer_realtime_updater::on_notify_failed(common::notification_ptr notification, int reason) 
{
	// TODO impl this
}

/**
 * Private file_info_buffer_realtime_updater api
 */

} } // namespace t2h_core, details

