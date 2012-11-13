#ifndef THREAD_POOL_HPP_INCLUDED
#define THREAD_POOL_HPP_INCLUDED

#include <vector>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <iostream>

namespace utility {

namespace details {

class default_policy {
public :
	typedef std::size_t size_type;

	default_policy() : lock_(), threads_(), thread_holder_() 
	{
	
	}
	
	explicit default_policy(size_type max_threads) 
		: lock_(), threads_(), thread_holder_() 
	{
		thread_holder_.resize(max_threads);
	} 

	~default_policy() 
	{
	}
	
	template <class Func>
	inline void add_task(size_type id, Func func) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		if (id < thread_holder_.size()) {
			if (thread_holder_.at(id) == NULL) {
				boost::thread * new_thread = threads_.create_thread(func);
				thread_holder_.at(id) = new_thread;
			} // !if
		} // !if
	}

	inline void join_task_and_remove(size_type id) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		boost::thread * thread_ptr = NULL;
		if (id < thread_holder_.size()) {
			if ((thread_ptr = thread_holder_.at(id)) != NULL) {
				thread_holder_.at(id) = NULL;
				thread_ptr->join();
				threads_.remove_thread(thread_ptr);
			} // !if 
		} // !if
	}
	
	inline void stop_all_tasks() 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		threads_.interrupt_all();
		clear_task_holder();
	}

	inline void wait_all_tasks() 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		threads_.join_all();
		clear_task_holder();
	}

	bool has_unfinished_task() const 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		bool state = false;
		std::vector<boost::thread *>::const_iterator 
			it = thread_holder_.begin(), end = thread_holder_.end();
		for (; it != end; ++it) {
			if (*it != NULL) 
				state = (*it)->joinable();
		}
		return state;
	}

	inline void resize_max_threads(size_type max_threads) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		std::vector<boost::thread *> old_holder = thread_holder_;
		clear_task_holder();
		thread_holder_.resize(max_threads);
		thread_holder_.swap(old_holder);
	}


private :
	inline void clear_task_holder() 
	{
		size_type const holder_size =  thread_holder_.size();
		thread_holder_.clear();
		thread_holder_.resize(holder_size);
	}

	boost::mutex mutable lock_;	
	boost::thread_group threads_;
	std::vector<boost::thread *> thread_holder_;

};

} // namespace details

template <class Policy = details::default_policy>
class base_thread_pool : private boost::noncopyable {
public :
	typedef Policy policy_type;
	typedef typename Policy::size_type size_type;

	base_thread_pool() : impl_policy_() 
	{
	}

	explicit base_thread_pool(size_type max_threads) 
		: impl_policy_(max_threads) 
	{ 
	}
	
	~base_thread_pool() 
	{
		wait_all_tasks();	
	}
	
	template <class Func>
	inline void add_task(size_type id, Func func) 
	{
		impl_policy_.add_task<Func>(id, func);	
	}

	inline void join_task_and_remove(size_type id) 
	{
		impl_policy_.remove_task_and_join(id);
	}
	
	inline void stop_all_tasks() 
	{
		impl_policy_.stop_all_tasks();
	}

	inline void wait_all_tasks() 
	{
		impl_policy_.wait_all_tasks();	
	}

	inline bool has_unfinished_task() const 
	{
		return impl_policy_.has_unfinished_task();
	}

	inline void resize_max_threads(size_type max_threads) 
	{
		impl_policy_.resize_max_threads(max_threads);
	}

private :
	policy_type impl_policy_;

};

} // namespace utility

#endif

