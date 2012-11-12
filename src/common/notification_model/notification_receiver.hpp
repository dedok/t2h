#ifndef BASE_NOTIFICATIO_RECEIVER_HPP_INCLUDED
#define BASE_NOTIFICATIO_RECEIVER_HPP_INCLUDED

#include "base_notification.hpp"

#include <string>
#include <boost/shared_ptr.hpp>

namespace common {

/**
 * notification_receiver
 */
class notification_receiver {
public :
	typedef boost::shared_ptr<notification_receiver> ptr_type;

	notification_receiver() : name_() { }
	explicit notification_receiver(std::string const & name) : name_(name) { }	
	virtual ~notification_receiver() { }
	
	virtual void on_notify(notification_ptr notification) = 0;	
	virtual void on_notify_failed(notification_ptr notification, int reason) = 0;

	virtual void set_name(std::string const & name) 
		{ name_ = name;  }	
	virtual std::string const & get_name() const 
		{ return name_; }

private :
	std::string mutable name_;

};

typedef notification_receiver::ptr_type notification_receiver_ptr;

} // namespace common

#endif

