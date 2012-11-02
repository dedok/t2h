#ifndef TORRENT_CORE_EVENT_HANDLER_HPP_INCLUDED
#define TORRENT_CORE_EVENT_HANDLER_HPP_INCLUDED

#include <string>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

namespace t2h_core { 

class torrent_core_event_handler {
public :
	torrent_core_event_handler() { }
	virtual ~torrent_core_event_handler() { }
	
	/**
	 * Good notifications
	 */
	virtual void on_file_add(std::string const & file_path, boost::int64_t file_size) = 0;
	virtual void on_file_remove(std::string const & file_path) = 0;

	virtual void on_file_complete(std::string const & file_path, boost::int64_t avaliable_bytes) = 0; 
	virtual void on_pause(std::string const & file_path) = 0;
	
	virtual void on_progress_update(std::string const & file_path, boost::int64_t avaliable_bytes) = 0;

	/**
	 * Bad notifications
	 */
private :

};

typedef boost::shared_ptr<torrent_core_event_handler> torrent_core_event_handler_ptr;

} // namespace t2h_core

#endif

