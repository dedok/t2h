#ifndef CORE_NOTIFICATION_CENTER_HPP_INCLUDED
#define CORE_NOTIFICATION_CENTER_HPP_INCLUDED

#include "notification_center.hpp"

#include <boost/noncopyable.hpp>

namespace t2h_core {


static inline common::notification_center_ptr core_notification_center() 
{
	static common::notification_center_config config = { 1000, false };
	static common::notification_center_ptr center(new common::notification_center(config));
	return center;
}

} // namespace t2h_core

#endif

