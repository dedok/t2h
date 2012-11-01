#include "http_transport_ev_handler.hpp"
#include "hcore_notification_recv.hpp"
#include "core_file_change_notification.hpp"

namespace t2h_core {

hcore_notification_recv::hcore_notification_recv(transport_ev_handler & handler) 
	: common::notification_receiver("hcore_notification_recv"), handler_(handler) 
{ 
}

hcore_notification_recv::~hcore_notification_recv() 
{
}
	
void hcore_notification_recv::on_notify(common::notification_ptr notification) 
{
	core_file_change_notification_ptr file_change_notification 
		= common::notification_cast<core_file_change_notification>(notification);
	if (file_change_notification) {
		switch (file_change_notification->event_type) {
			case core_file_change_notification::file_remove : 
				handler_.on_file_remove(file_change_notification->file_path);
			break;
			case core_file_change_notification::file_add :
				handler_.on_file_add(file_change_notification->file_path, 
					file_change_notification->file_size, file_change_notification->avaliable_bytes);
			break;
			case core_file_change_notification::file_update :
				handler_.on_file_update(file_change_notification->file_path, 
					file_change_notification->file_size, file_change_notification->avaliable_bytes);
			break;
		} // switch
		file_change_notification->set_state(common::base_notification::done);
	}
}

void hcore_notification_recv::on_notify_failed(common::notification_ptr notification, int reason) 
{
}

} // namespace 

