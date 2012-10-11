#ifndef HANDLES_MANAGER_HPP_INCLUDED
#define HANDLES_MANAGER_HPP_INCLUDED

#include <map>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace details {

/**
 * Glogal thread safe handle manager
 */
template <class UnderlaingHandle>
class handles_manager : boost::noncopyable {
public :
	typedef UnderlaingHandle underlying_handle;
	typedef boost::shared_ptr<handles_manager> handles_manager_ptr;
	typedef typename std::map<std::size_t, underlying_handle> underlying_handles_map_type;

	handles_manager() : lock_(), handles_map_() { }
	~handles_manager() { }

	inline static handles_manager_ptr shared_manager() 
	{
		static handles_manager_ptr manager(new handles_manager());
		return manager;
	}

	inline int registr_new_handle(underlying_handle handle) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		std::size_t const new_id = handles_map_.size() + 1;
		handles_map_[new_id] = handle;
		return new_id;
	}

	inline underlying_handle unregistr_handle(std::size_t handle_id) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		typename underlying_handles_map_type::iterator found = handles_map_.find(handle_id);
		if (found != handles_map_.end()) {
			underlying_handle handle = found->second;
			handles_map_.erase(found);
			return handle;
		}
		return underlying_handle();
	}

	inline underlying_handle get_handle(std::size_t handle_id) const
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		typename underlying_handles_map_type::const_iterator found = handles_map_.find(handle_id);
		return (found != handles_map_.end()) ? 
			found->second : underlying_handle();
	}

private :
	boost::mutex mutable lock_;
	underlying_handles_map_type handles_map_;

};

} // namespace details 

#endif 

