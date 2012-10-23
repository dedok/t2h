#ifndef HCORE_NOTIFICATION_RECV_HPP_INCLUDED
#define HCORE_NOTIFICATION_RECV_HPP_INCLUDED

#include "notification_receiver.hpp"
#include ""

class http_transport_ev_handler; 

namespace t2h_core {

class hcore_notification_recv : public common::notification_receiver {
public :
	explicit hcore_notification_recv(http_transport_ev_handler & handler);
	~hcore_notification_recv();
	
	virtual void on_notify(common::notification_ptr notification);	
	virtual void on_notify_failed(common::notification_ptr notification, int reason);

private :
	http_transport_ev_handler & handler_;
}; 

} // namespace t2h_core 

#endif

