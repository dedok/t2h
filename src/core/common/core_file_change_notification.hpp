#ifndef CORE_FILE_STATE_CHANGE_NOTIFICATION_HPP_INCLUDED
#define CORE_FILE_STATE_CHANGE_NOTIFICATION_HPP_INCLUDED

#include "base_notification.hpp"
#include <boost/cstdint.hpp>

namespace t2h_core {

struct core_file_change_notification : public common::base_notification {
	enum change_state {
		file_update,
		file_remove,
		file_add
	};

	core_file_change_notification() 
		: common::base_notification(__LINE__), state_(base_notification::executeable) { }

	virtual notification_state get_state() const  { return state_; }
	virtual void set_state(notification_state state) { state_ = state; }
	
	notification_state state_;
	change_state event_type; 
	std::string file_path;
	boost::int64_t file_size;
	boost::int64_t avaliable_bytes;
}; 

typedef boost::shared_ptr<core_file_change_notification> core_file_change_notification_ptr;

} // namespace t2h_core

#endif

