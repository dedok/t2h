#ifndef BASE_NOTIFICATION_HPP_INCLIDED
#define BASE_NOTIFICATION_HPP_INCLIDED

#include <boost/shared_ptr.hpp>

namespace common {

class base_notification {
public :
	typedef boost::shared_ptr<base_notification> ptr_type;

	/**
	 * notification_state current state of this notification
	 */
	enum notification_state {
		ignore = 0x0,			// this state mean ignore this notification(eg do not notify recv about this event)
		executeable = 0x1,		// this state mean this event are ready for execute(eg notify recv about this event)
		done = 0x2,				// this state set to notification after when notification is done
		unknown = done + 0x1	// stub
	};
	
	base_notification(int notification_type_) : notification_type(notification_type_) { }
	virtual ~base_notification() { }
	
	virtual notification_state get_state() const = 0;
	virtual void set_state(notification_state state) = 0;
	
	int const notification_type;	// notification type(do not set same values for difference notifications)
};

typedef base_notification::ptr_type notification_ptr;

/**
 *	cast notification_ptr to XXX_notification_ptr;
 */
template <class T, class U>
static inline boost::shared_ptr<T> notification_cast(boost::shared_ptr<U> & u) 
	{ return boost::static_pointer_cast<T>(u); }

} // namesace common

#endif

