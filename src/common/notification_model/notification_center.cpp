#include "notification_center.hpp"

namespace common {

/**
 * Public notification_center api
 */

notification_center::notification_center(notification_center_config const & config) 
	: config_(config), receivers_(), lock_(), stop_(false) { } 

notification_center::~notification_center() 
{  
	boost::lock_guard<boost::mutex> guard(lock_);
}

void notification_center::change_configuration(notification_center_config const & config) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	config_ = config;
}

boost::tuple<std::size_t, bool> 
	notification_center::add_notification_receiver(notification_receiver_ptr receiver) 
{
	bool state = false;
	std::size_t object_id = 0;

	{ // receivers_ lock zone
	boost::lock_guard<boost::mutex> guard(lock_);
	if (receiver && !stop_) {
		if (receivers_.end() == receivers_.find(receiver->get_name())) {
			details::notification_unit_param p = 
			{ 
				true, 
				config_.execute_notification_at_recv_gone, 
				3000, 
				config_.max_notifications_per_recv, 
				receiver  
			}; 
			details::notification_unit_ptr new_notification_unit(new details::notification_unit(p));
			receivers_.insert(std::make_pair(receiver->get_name(), new_notification_unit));
			object_id = new_notification_unit->get_id();
			new_notification_unit->execution_loop_run();
			state = true;
		}
	} // receiver
	} // receivers_ lock zone end

	return boost::make_tuple(state, object_id);
}

void notification_center::remove_notification_receiver(std::string const & name) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	if (!stop_) {
		units_type::iterator found = receivers_.find(name);
		if (receivers_.end() != found)  
			receivers_.erase(found);
	} // stop_
}

void notification_center::remove_notification_receiver(std::size_t id) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	/* TODO impl this */
}

bool notification_center::send_message(std::string const & recv_name, notification_ptr notification) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	if (!stop_ && notification) {
		units_type::iterator found = receivers_.find(recv_name);
		if (receivers_.end() != found) {
			found->second->add_notification(notification);
			return true;
		} // found
	}
	return false;

}

bool notification_center::post_message(std::string const & recv_name, notification_ptr notification)
{
	boost::lock_guard<boost::mutex> guard(lock_);
	if (!stop_ && notification) {
		units_type::iterator found = receivers_.find(recv_name);
		if (receivers_.end() != found) {
			found->second->add_notification(notification);
			return true;
		} // found
	}
	return false;
}

/**
 * Private notification_center api
 */

} // namespace common

