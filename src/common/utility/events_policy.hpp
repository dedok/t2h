#ifndef EVENTS_POLICY_HPP_INCLUDED
#define EVENTS_POLICY_HPP_INCLUDED

#include "event_wrapper.hpp"

#include <deque>
#include <limits>
#include <algorithm>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/condition.hpp>
#include <boost/enable_shared_from_this.hpp> 

#include <iostream>

#define BEGIN_GUARD_SCOPE(x) { \
	boost::lock_guard<boost::mutex> _guard(x);

#define END_GUARD_SCOPE }

namespace event_policy {

template <class T>
struct event_tuple {
	T queue;
	bool need_progress;
	boost::mutex mutable lock;
};

template <class EventWrapper > 
class exchange_events_queue : boost::noncopyable {
public :
	
	typedef EventWrapper event_type;		
	typedef typename event_type::event_ptr event_ptr;
	typedef typename event_type::object_type object_type;
	typedef typename event_type::function_type function_type;	

	typedef std::deque<event_ptr> queue_type;
	typedef typename queue_type::size_type size_type;	
	typedef event_tuple<queue_type> queue_tuple_type;

	exchange_events_queue() 
		: exec_queue_0(), exec_queue_1(), thread_loop_(), cond_()
	{
		exec_queue_0.need_progress = exec_queue_1.need_progress = thread_loop_.is_run = false;
		thread_loop_.message_count = 0;
	}

	~exchange_events_queue() 
	{ 
		stop_poll();
		wait();
	}

	/**
	 * Post event to execution
	 */
	inline void post_event(function_type const & func, object_type object) 
	{	
		boost::lock_guard<boost::mutex> guard(thread_loop_.lock);	
		if (!thread_loop_.is_run) 
			return;
		boost::lock_guard<boost::mutex> queue_guard(cond_.lock);
		exec_queue_.queue.push_back(event_type::new_event(func, object));	
		exec_queue_.need_progress = true;
		cond_.cond.notify_one();
		cond_.lock.unlock();
		
		++thread_loop_.message_count;
	}
	
	/**
	 * Run execution loop not in main thread, at set queue in the 'run mode'
	 */
	inline void poll() 
	{
		boost::lock_guard<boost::mutex> guard(thread_loop_.lock);	
		if (!thread_loop_.loop) {
			boost::thread * loop_ptr = new boost::thread(
				&exchange_events_queue<EventWrapper>::non_schedule_loop, this);
			thread_loop_.loop.reset(loop_ptr);
			thread_loop_.is_run = true;
		}
	}
	
	/**
	 * Get not executed messages in queue
	 */
	inline std::size_t get_message_count() const 
	{ 
		boost::lock_guard<boost::mutex> guard(thread_loop_.lock);
		return thread_loop_.message_count;
	} 

	/** 
	 * Test run mode 
	 */
	inline bool is_runing() const 
	{
		boost::lock_guard<boost::mutex> guard(thread_loop_.lock);
		return thread_loop_.is_run;
	}
	
	/** 
	 * Notify polling thread about stop without end of execution waiting 
	 */
	inline void stop_poll() 
	{
		boost::lock_guard<boost::mutex> guard(thread_loop_.lock);
		if (thread_loop_.is_run) {
			thread_loop_.is_run = false;
			boost::lock_guard<boost::mutex> cond_guard(cond_.lock);
			cond_.cond.notify_one();
		}
	}
	
	/**
	 * Wait for all message executed, then this function return control, 
	 * but only if stop_poll called otherwise this function will block call thread.
	 * By default behavior in dtor calling stop_poll then wait.
	 */
	inline void wait() 
	{
		boost::lock_guard<boost::mutex> guard(thread_loop_.lock);
		if (thread_loop_.loop) {
			boost::lock_guard<boost::mutex> cond_guard(cond_.lock);
			cond_.cond.notify_one();
			thread_loop_.loop->join();
			thread_loop_.loop.reset();
		}
	}

private :	
	/**
	 * main execution loop
	 */
	void non_schedule_loop() 
	{
		/** wait for notification about new message, after execute all message in exec_queue,
			all new message will placed to exchange_queue */	
		boost::unique_lock<boost::mutex> guard(cond_.lock);
		for (;;) {	
			cond_.cond.wait(guard);
			if ((selected_queue = select_queue()) != NULL) {
				execute_queue(*selected_queue);
				selected_queue->lock.unlock();
				selected_queue->need_progress = false;
			} // !if
			if (!is_runing())  
				break; 
		} // !for
	}
	
	inline void execute_all_queue() 
	{
		execute_queue(exec_queue);	
	}

	/**
	 * sync execute execute_queue
	 */
	void execute_queue(queue_tuple_type & tuple) 
	{
		event_ptr event_to_exec;
		while (!tuple.queue.empty()) {
			event_to_exec = tuple.queue.front();
			tuple.queue.pop_front();
			decrease_message_size();
			event_to_exec->event_execute();
		}
	}

	queue_tuple_type exec_queue;

	/** working thread, and states */
	struct {	
	boost::scoped_ptr<boost::thread> loop;
	boost::mutex mutable lock;
	std::size_t message_count;
	bool mutable is_run;
	} thread_loop_;

	/** condition variable for blocking work thread */	
	struct {
	boost::condition cond;
	boost::mutex lock;
	} cond_;
};

} // !event_policies

#undef BEGIN_GUARD_SCOPE 

#undef END_GUARD_SCOPE 

#endif

