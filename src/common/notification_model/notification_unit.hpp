#ifndef NOTIFICATION_UNIT_HPP_INCLUDED
#define NOTIFICATION_UNIT_HPP_INCLUDED

#include "notification_receiver.hpp"

#include <list>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace common { namespace details {

/**
 * notification_unit_param need for valid initialize of notification_unit
 */
struct notification_unit_param {
	bool change_state_auto;						// change state after notification return controll
	bool execute_all_notification_at_exit;		// execute all events in queue before retur control from notification_unit::dtor
	std::size_t notification_check_timeout;		// test queue timeout in MS
	std::size_t max_notifications;				// max notification in queue
	notification_receiver_ptr recv;				// notification receiver
};

/**
 * notification_unit provide wrapped notification loop under receiver
 */
class notification_unit {
public :
	typedef boost::shared_ptr<notification_unit> ptr_type;
	
	explicit notification_unit(notification_unit_param const & param);
	~notification_unit();

	void add_notification(notification_ptr notification);
	void execution_loop_run();
	
	inline std::string const & get_name() const 
		{ return unit_name_; }

	inline std::size_t get_id() const
		{ return unit_id_; }

	inline notification_unit_param const & get_param() const 
		{ return p_; }

private :
	typedef std::list<notification_ptr> notifications_type;

	void execution_loop();
	bool copy_pending_notifications_unsafe();
	
	inline bool copy_pending_notifications() 
	{ 
		boost::lock_guard<boost::mutex> guard(pn_lock_); 
		return copy_pending_notifications_unsafe(); 
	}

	void notify_receiver();
	
	std::string mutable unit_name_;
	std::size_t mutable unit_id_;
	
	notifications_type notifications_;
	notifications_type pending_notifications_;
	
	boost::mutex mutable lock_;
	boost::mutex mutable pn_lock_;
	boost::condition_variable waiters_;
	boost::thread * execution_loop_thread_;
	bool stop_work_;

	notification_unit_param p_;
};

typedef notification_unit::ptr_type notification_unit_ptr;

} } // namespace common, details 

#endif

