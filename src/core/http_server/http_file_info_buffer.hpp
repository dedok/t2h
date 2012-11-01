#ifndef HTTP_FILE_INFO_BUFFER_HPP_INCLUDED
#define HTTP_FILE_INFO_BUFFER_HPP_INCLUDED

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

namespace t2h_core { namespace details {

/**
 *
 */
struct hc_file_info {
	std::string file_path;						// 
	boost::int64_t file_size;					//
	boost::int64_t avaliable_bytes;				//
	boost::mutex mutable waiter_lock;			//
	boost::condition_variable mutable waiter;	//
};

typedef boost::shared_ptr<hc_file_info> hc_file_info_ptr;

/**
 *
 */
class file_info_buffer : boost::noncopyable {
public :
	typedef boost::unordered_map<std::string, hc_file_info_ptr> infos_type;
	
	file_info_buffer();
	~file_info_buffer();
	
	bool wait_avaliable_bytes(
		std::string const & file_path, boost::int64_t avaliable_bytes, std::size_t seconds);
	void remove_info(std::string const & path);
	void update_info(hc_file_info_ptr info);
	
	hc_file_info_ptr get_info(std::string const & path) const;
	
private :
	boost::mutex mutable lock_;
	infos_type infos_;

};

} } // namespace t2h_core, details

#endif

