#ifndef EVENT_WRAPPER_HPP_INCLUDED
#define EVENT_WRAPPER_HPP_INCLUDED

#include <utility>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

template <class T>
class event_wrapper {
public :
	typedef T object_type;
	typedef boost::shared_ptr<event_wrapper<T> > event_ptr;
	typedef boost::function<void (object_type)> function_type;
	
	event_wrapper() : event_(), object_(NULL) 
	{
	}
	
	event_wrapper(function_type const & event, object_type object) 
		: event_(event), object_(object)
	{ 
	}

	/** named constructor(s) with reference counting */
	template <class CountObject>
	static inline event_ptr new_event(
		function_type const & func, object_type obj, CountObject & count) 
	{
		event_ptr event(new event_wrapper<T>(func, obj)); 
		++count;	
		return event;
	}

	/** static mem funtion(s) with reference counting */	
	template <class CountObject>
	static inline void swap(
		event_ptr & first, event_ptr & second, CountObject & first_count) 
	{
		std::swap(first, second);
		++first_count; 
	}

	template <class CountObject>
	static inline void release(event_ptr & event, CountObject & count) 
	{
		event.reset();
		--count;
	}
	
	/** public(s) mem funtion(s) without ref counting */
	inline void set_event(function_type const & event, object_type object) 
	{
		event_ = event;
		object_ = object_; 
	}
		
	inline void event_execute() 
	{
		if (object_) 
			event_(object_);
	}

private :
	/** no value semantics, copy only via shared_ptr(share_ptr & rhs) */
	event_wrapper(event_wrapper const &);
	event_wrapper & operator=(event_wrapper const &);

	function_type event_;
	object_type mutable object_;

};

template <class T>
class shared_event_wrapper {
public :
	typedef boost::shared_ptr<T> object_type;
	typedef T * element_type;
	typedef boost::shared_ptr<shared_event_wrapper<T> > event_ptr;
	typedef boost::function<void ()> function_type;
	
	shared_event_wrapper() : event_(), object_(NULL) 
	{
	}
	
	shared_event_wrapper(function_type const & event, object_type object) 
		: event_(event), object_(object)
	{ 
	}

	/** named constructor(s) with reference counting */
	static inline event_ptr new_event(
		function_type const & func, object_type obj) 
	{
		event_ptr event(new shared_event_wrapper<T>(func, obj)); 
		return event;
	}

	/** static mem funtion(s) with reference counting */	
	static inline void swap(
		event_ptr & first, event_ptr & second) 
	{
		std::swap(first, second);
	}

	static inline void release(event_ptr & event) 
	{
		event.reset();
	}
	
	/** public(s) mem funtion(s) without ref counting */
	inline void set_event(function_type const & event, object_type object) 
	{
		event_ = event;
		object_ = object_; 
	}
		
	inline void event_execute() 
	{
		if (object_) {
			event_();
			object_.reset();
		}
	}

private :
	/** no value semantics, copy only via shared_ptr(share_ptr & rhs) */
	shared_event_wrapper(shared_event_wrapper const &);
	shared_event_wrapper & operator=(shared_event_wrapper const &);

	function_type event_;
	object_type object_;

};

#endif

