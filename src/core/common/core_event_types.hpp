#ifndef CORE_EVENT_TYPES_HPP_INCLUDED
#define CORE_EVENT_TYPES_HPP_INCLUDED

#include "base_notification.hpp"

#include <boost/cstdint.hpp>

namespace t2h_core {

/**
 *
 */
namespace details {

struct e_file_info {
	enum file_state {
		downloading = 0x0,
		done,
		file_error,
	};
	boost::int64_t bytes_avaliable;
	boost::int64_t size;
	std::string file_path;
};

struct e_error {

};

} // namespace details

/**
 *
 */
struct file_info_event : public common::base_notification {
	explicit file_info_event(file_info const & info) 
		: common::base_notification(__LINE__), file_info(info) { }
	e_file_info file_info;
};

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

