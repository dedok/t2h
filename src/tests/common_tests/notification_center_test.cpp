#include "notification_center.hpp"

#include <iostream>

class not_t1 : public common::base_notification {
public :
    not_t1() : common::base_notification(__LINE__) { }
	virtual notification_state get_state() const { return state_; }
	virtual void set_state(notification_state state) { state_ = state; }

	notification_state state_;
	struct {
		int c;
	} udata;
};

typedef boost::shared_ptr<not_t1> not_t1_ptr;


class recv_t1 : public common::notification_receiver {
public :
	recv_t1() : common::notification_receiver("recv_t1") { }
	
	virtual void on_notify(common::notification_ptr notification)
	{ 
		if (notification) { 
			not_t1_ptr ev = common::notification_cast<not_t1>(notification);
			std::cout << ev->udata.c << std::endl;
		} else std::cerr << "notification == NULL" << std::endl;
	}

private :
};

void die(std::string const & message, int exit_code) 
{
	std::cerr << "Error detected : " << message << std::endl;
	std::exit(exit_code);
}

int main(int argc, char ** argv) 
{
	common::notification_center_config ncc = { 10, true, true };	
	common::notification_center nc(ncc);
	
	common::notification_receiver_ptr recv(new recv_t1());
	bool state; std::size_t id;
	boost::tie(id, state) = nc.add_notification_receiver(recv);
	if (!state)
		die("Add recv failed", 1);

	for (int count = 0; ; ++count ) {
		std::cout << "send new notifycation ..." << std::endl;
		not_t1 * n = new not_t1();
		n->udata.c = count;
		common::notification_ptr event(n);
		nc.send_message(recv->get_name(), event);
	}

	return 0;
}

