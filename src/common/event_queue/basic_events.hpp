#ifndef BASIC_EVENTS_HPP_INCLUDED
#define BASIC_EVENTS_HPP_INCLUDED

#include <utility>
#include <boost/thread.hpp>

template <class EventPolicy> class basic_event_source;
template <class EventPolicy> class basic_event_binder;

template <class EventPolicy> 
class basic_event_binder {
public:
	typedef EventPolicy policy_type;
	typedef typename policy_type::sink_type sink_type;
	typedef basic_event_source<policy_type> event_source_type;

	basic_event_binder() 
		: sink_(sink_type()), locker_(), prev_(NULL), next_(NULL) 
	{ 
		boost::lock_guard<boost::mutex> guard(locker_);
		next_ = prev_ = this; 
	}

	virtual ~basic_event_binder() { 
		unbind(); 
	}

	void bind(event_source_type const & source, sink_type sink);
		
	inline void unbind() {
		boost::lock_guard<boost::mutex> guard(locker_);
		prev_->next_ = next_;
		next_->prev_ = prev_;
		next_ = prev_ = this;
	}

private:		
	friend class basic_event_source<policy_type>;

	basic_event_binder(basic_event_binder const &);
	basic_event_binder & operator=(basic_event_binder const&);

	inline void unbind_weak() {
		prev_->next_ = next_;
		next_->prev_ = prev_;
		next_ = prev_ = this;
	}

	inline void attach_after(basic_event_binder * what) {
		next_ = what->next_;
		next_->prev_ = this;
		what->next_ = this;
		prev_ = what;
	}
	
	sink_type sink_;
	boost::mutex mutable locker_;
	basic_event_binder * prev_;
	basic_event_binder * next_;
};

// TODO add thread safety
template <class EventPolicy> 
class basic_event_source {
public:
	typedef EventPolicy policy_type;
	typedef typename policy_type::sink_type sink_type;	
	typedef typename policy_type::function_type function_type;
	
	typedef basic_event_binder<policy_type> binder_type;

	basic_event_source() 
		: policy_(), list_head_() 
	{
		boost::lock_guard<boost::mutex> guard(basic_event_locker_);
		policy_.poll(); 
	}

	virtual ~basic_event_source() { 
		boost::lock_guard<boost::mutex> guard(basic_event_locker_);
		policy_.stop_poll(); 
	}

	inline void bind(binder_type & bndr, sink_type sink) {
		boost::lock_guard<boost::mutex> guard(basic_event_locker_);
		bndr.attach_after(&list_head_);
		bndr.sink = sink;
	}

	template <class Invoker>
	void post_event(Invoker const & invoker) {
		boost::lock_guard<boost::mutex> guard(basic_event_locker_);
		binder_type * current = list_head_.next_;
		while (current != &list_head_) {
			if (current->sink_) {
				binder_type bookmark;
				bookmark.attach_after(current);
				policy_.post_event(invoker, current->sink_); 	
				current = bookmark.next_;
				continue;
			}
			current = current->next_;
		}
	}

private:		
	friend class basic_event_binder<policy_type>;
	
	basic_event_source(basic_event_source const &);
	basic_event_source& operator=(basic_event_source const &);

	boost::mutex mutable basic_event_locker_;
	policy_type policy_;
	binder_type mutable list_head_;

};

template <class EventPolicy>
void basic_event_binder<EventPolicy>::bind(
	event_source_type const & source, sink_type sink) 
{
	boost::lock_guard<boost::mutex> guard(locker_);
	boost::lock_guard<boost::mutex> guard_basic(source.basic_event_locker_);
	unbind_weak();
	attach_after(&source.list_head_);
	this->sink_ = sink;
}

#endif

