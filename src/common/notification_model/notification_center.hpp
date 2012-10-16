#ifndef NOTIFICATION_CENTER_HPP_INCLUDED
#define NOTIFICATION_CENTER_HPP_INCLUDED

#include "notification_unit.hpp"

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

namespace common {

/**
 *
 */
class notification_center_exception : public std::exception {
public :
	explicit notification_center_exception(std::string const & message) 
		: std::exception(), message_(message) { }
	virtual ~notification_center_exception() throw () { }

	virtual char const * what() const throw () 
		{ return message_.c_str(); }

private :
	std::string const message_;

};

/**
 * Configuration for the notification_center object
 */
struct notification_center_config {
	std::size_t max_notifications_per_recv;		// max pending notification per receiver
	bool execute_notification_at_recv_gone;		// on/off execute all notification at exit/remove
};

/**
 *
 */
class notification_center : public boost::noncopyable {
public :
	typedef boost::shared_ptr<notification_center> ptr_type;

	explicit notification_center(notification_center_config const & config);
	~notification_center();
	
	void change_configuration(notification_center const & config);

	boost::tuple<std::size_t, bool> add_notification_receiver(notification_receiver_ptr receiver);
	void remove_notification_receiver(std::size_t id);
	void remove_notification_receiver(std::string const & name);

	bool send_message(std::string const & recv_name, notification_ptr notification);
	bool post_message(std::string const & recv_name, notification_ptr notification);

private :
	typedef boost::unordered_map
	<
		std::string, 
		details::notification_unit_ptr
	> 
	units_type;
	
	notification_center_config mutable config_;
	units_type receivers_;
	boost::mutex mutable lock_;
	bool stop_;

};

typedef notification_center::ptr_type notification_center_ptr;

} // namespace common

#endif

