#include "hc_event_source_adapter.hpp"

#include "core_notification_center.hpp"
#include "core_file_change_notification.hpp"

#define SEND_NOTIFICATION(recv_name, n) 								\
do {																	\
	notification_center_->send_message(recv_name, n);					\
}while(0);																

namespace t2h_core { namespace details {

/**
 * Public hc_event_source_adapter api
 */
hc_event_source_adapter::hc_event_source_adapter() 
	: torrent_core_event_handler(), 
	recv_name_("hcore_notification_recv"),
	notification_center_(core_notification_center())
{ 
}

hc_event_source_adapter::~hc_event_source_adapter() 
{ 
}

void hc_event_source_adapter::on_file_add(std::string const & file_path, boost::int64_t file_size) 
{
	core_file_change_notification_ptr add_notification(new core_file_change_notification());
	add_notification->event_type = core_file_change_notification::file_add;
	add_notification->file_path = file_path;
	add_notification->file_size = file_size;
	add_notification->avaliable_bytes = 0;
	SEND_NOTIFICATION(recv_name_, add_notification)
}

void hc_event_source_adapter::on_file_remove(std::string const & file_path) 
{
	core_file_change_notification_ptr remove_notification(new core_file_change_notification());
	remove_notification->event_type = core_file_change_notification::file_remove;
	remove_notification->file_path = file_path;
	remove_notification->file_size = remove_notification->avaliable_bytes = 0;
	SEND_NOTIFICATION(recv_name_, remove_notification)
}

void hc_event_source_adapter::on_file_complete(std::string const & file_path, boost::int64_t avaliable_bytes) 
{
	on_progress_update(file_path, avaliable_bytes);
}

void hc_event_source_adapter::on_pause(std::string const & file_path) 
{
	// TODO implement this
}
	
void hc_event_source_adapter::on_progress_update(std::string const & file_path, boost::int64_t avaliable_bytes) 
{
	core_file_change_notification_ptr update_notification(new core_file_change_notification());
	update_notification->event_type = core_file_change_notification::file_update;
	update_notification->file_path = file_path;
	update_notification->avaliable_bytes = avaliable_bytes;
	SEND_NOTIFICATION(recv_name_, update_notification)
}

} } // namespace t2h_core, details

