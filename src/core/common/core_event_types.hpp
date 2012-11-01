#ifndef CORE_EVENT_TYPES_HPP_INCLUDED
#define CORE_EVENT_TYPES_HPP_INCLUDED

#include "base_notification.hpp"

#include <boost/cstdint.hpp>

namespace t2h_core {

/**
 *
 */
struct error_event : public common::base_notification {
	explicit file_info_event(e_error const & e) 
		: common::base_notification(__LINE__), error(e) { }
	e_error error;
};

typedef boost::shared_ptr<core_event> core_event_ptr;

} // namespace t2h_core

#endif

