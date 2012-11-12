#ifndef FILE_INFO_BUFFER_HPP_INCLUDED
#define FILE_INFO_BUFFER_HPP_INCLUDED

#include "notification_receiver.hpp"
#include "core_file_change_notification.hpp"

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

namespace t2h_core { namespace details {

/**
 * file_info_buffer item, contain useful information for syncing/getting information about files(real-time)
 */
struct hc_file_info {
	std::string file_path;						// Path to file(this use as key to find hc_file_info) 
	boost::int64_t file_size;					// File size(real)
	boost::int64_t avaliable_bytes;				// Current file_size
	boost::mutex mutable waiter_lock;			// Waiter for wait_avaliable_bytes(notified via add/update)
	boost::condition_variable mutable waiter;	// Waiter lock
};

typedef boost::shared_ptr<hc_file_info> hc_file_info_ptr;

/**
 * file_info_buffer 
 */
class file_info_buffer : boost::noncopyable {
public :
	typedef boost::unordered_map<std::string, hc_file_info_ptr> infos_type;
	
	file_info_buffer();
	~file_info_buffer();
	
	bool wait_avaliable_bytes(
		std::string const & file_path, boost::int64_t avaliable_bytes, std::size_t seconds);
	inline bool wait_avaliable_bytes(
		hc_file_info_ptr fi, boost::int64_t avaliable_bytes, std::size_t seconds) 
		{ wait_avaliable_bytes(fi->file_path, avaliable_bytes, seconds); }

	void remove_info(std::string const & path);
	
	void update_info(hc_file_info_ptr info);
	void update_info(std::string const & file_path, boost::int64_t avaliable_bytes);

	hc_file_info_ptr get_info(std::string const & path) const;
	
	inline void on_file_add(
		std::string const & file_path, 
		boost::int64_t file_size, 
		boost::int64_t avaliable_bytes) 
	{
		hc_file_info_ptr finfo(new hc_file_info()); 
		finfo->file_path = file_path; 
		finfo->file_size = file_size;
		finfo->avaliable_bytes = avaliable_bytes;
		update_info(finfo);
	}

	inline void on_file_remove(std::string const & file_path) 
		{ remove_info(file_path); }	

	inline void on_file_update(
		std::string const & file_path, 
		boost::int64_t file_size, 
		boost::int64_t avaliable_bytes) 
		{ update_info(file_path, avaliable_bytes); }

private :
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

