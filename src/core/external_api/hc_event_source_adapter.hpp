#ifndef HC_EVENT_SOURCE_ADAPTER_HPP_INCLUDED
#define HC_EVENT_SOURCE_ADAPTER_HPP_INCLUDED

#include "torrent_core_event_handler.hpp"
#include "core_notification_center.hpp"

namespace t2h_core { namespace details {

/**
 *
 */
class hc_event_source_adapter : public torrent_core_event_handler {
public :
	hc_event_source_adapter();
	virtual ~hc_event_source_adapter();
	
	virtual void on_file_add(std::string const & file_path, boost::int64_t file_size);
	virtual void on_file_remove(std::string const & file_path);

	virtual void on_file_complete(std::string const & file_path, boost::int64_t avaliable_bytes); 
	virtual void on_pause(std::string const & file_path);
	
	virtual void on_progress_update(std::string const & file_path, boost::int64_t avaliable_bytes);

private :
	std::string const recv_name_;
	common::notification_center_ptr notification_center_;

};

} } // namespace t2h_core, details

#endif

