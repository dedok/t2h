#ifndef FILE_INFO_BUFFER_HPP_INCLUDED
#define FILE_INFO_BUFFER_HPP_INCLUDED

#include "notification_receiver.hpp"
#include "async_file_info_subscriber.hpp"
#include "core_file_change_notification.hpp"

#include <vector>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

namespace t2h_core { namespace details {

/**
 * hc_file_info is file_info_buffer item, contain useful information for 
 * syncing/getting information about files(real-time)
 */
struct hc_file_info : boost::noncopyable {
	hc_file_info() 
		: file_path(""), file_size(0), avaliable_bytes(0), subscribers()
	{ 
	}

	hc_file_info(
		std::string const & file_path_, boost::int64_t file_size_, boost::int64_t avaliable_bytes_) :
		file_path(file_path_), 
		file_size(file_size_), 
		avaliable_bytes(avaliable_bytes_),
		subscribers()
	{ 
	}
			
	std::string file_path;										// Path to file(this use as key to find hc_file_info) 
	boost::int64_t file_size;									// File size(real)
	boost::int64_t avaliable_bytes;								// Current file_size	
	std::vector<async_file_info_subscriber_ptr> subscribers;	// list of subscribers
};

typedef boost::shared_ptr<hc_file_info> hc_file_info_ptr;

template <class F>
inline void hc_file_info_notify_subscribers(hc_file_info_ptr fi, F f) 
	{ std::for_each(fi->subscribers.begin(), fi->subscribers.end(), f); }

/**
 * file_info_buffer 
 */
class file_info_buffer : boost::noncopyable {
public :
	typedef boost::unordered_map<std::string, hc_file_info_ptr> infos_type;
	
	file_info_buffer();
	~file_info_buffer();
	
	/**
	 * Controlling api
	 */
	int registr_subscriber(hc_file_info_ptr fi, async_file_info_subscriber_ptr subscriber);
	void unregistr_subscriber(hc_file_info_ptr fi, async_file_info_subscriber_ptr subscriber);
	void unregistr_subscriber(hc_file_info_ptr fi, int sid);

	hc_file_info_ptr get_info(std::string const & path) const;
	void update_info(std::string const & file_path, boost::int64_t avaliable_bytes);
	void remove_info(std::string const & path);	

	inline void stop_graceful() 
		{ close(true); }
	inline void stop_force() 
		{ close(false); }

	/**
	 * Callbacks for the notification reciever
	 */
	inline void on_file_add(
		std::string const & file_path, 
		boost::int64_t file_size, 
		boost::int64_t avaliable_bytes) 
	{ 
		// do not overwrite item at file_path exists just update data 
		boost::lock_guard<boost::mutex> guard(lock_);
		infos_type::iterator f; 
		if ((f = infos_.find(file_path)) != infos_.end()) {
			f->second->file_size = file_size;
			f->second->avaliable_bytes = avaliable_bytes;
			return;
		}
		infos_[file_path].reset(new hc_file_info(file_path, file_size, avaliable_bytes)); 
	}

	inline void on_file_remove(std::string const & file_path) 
		{ remove_info(file_path); }	

	inline void on_file_update(
		std::string const & file_path, 
		boost::int64_t file_size, 
		boost::int64_t avaliable_bytes) 
		{ update_info(file_path, avaliable_bytes); }
	
	inline void stop_graceful() 
		{ stop(true); }
	inline void stop_force() 
		{ stop(false); }

private :
	void stop(bool graceful);
		
	bool volatile mutable is_stoped_;
	boost::mutex mutable lock_;
	
	infos_type infos_;
	struct {
		std::string mutable recv_name;
		common::notification_receiver_ptr nr;
	} updater_;

};

typedef boost::shared_ptr<file_info_buffer> file_info_buffer_ptr;

/**
 * Public api
 */
file_info_buffer_ptr shared_file_info_buffer();

} } // namespace t2h_core, details

#endif

