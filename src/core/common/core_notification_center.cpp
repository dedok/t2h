#include "core_notification_center.hpp"

namespace t2h_core {

namespace details {
	static common::notification_center_config config = { 1000, false };
	static common::notification_center_ptr center;
}

common::notification_center_ptr core_notification_center() 
{
	if (!details::center)
		details::center.reset(new common::notification_center(details::config));
	return details::center;
}

}

