#ifndef BASIC_SAFE_CONTAINER_HPP_INCLUDED
#define BASIC_SAFE_CONTAINER_HPP_INCLUDED

#include <utility>
#include <algorithm>
#include <boost/thread.hpp>

namespace utility {

template <class Traits>
struct basic_safe_container {
	typedef typename Traits::object_type object_type;

	typedef typename Traits::item_type item_type;
	typedef typename Traits::item_ptr item_ptr; 
	typedef typename Traits::item_ref item_ref;
	typedef typename Traits::item_cref item_cref;

	typedef typename object_type::iterator iterator_type;
	typedef typename object_type::const_iterator const_iterator_type;	
	
	typedef typename Traits::find_type find_type;
	
	object_type cont;
	boost::mutex lock;
};

template <class Traits>
static inline 
typename basic_safe_container<Traits>::iterator_type
container_find_unsafe(
		basic_safe_container<Traits> & object_ref, 
		typename basic_safe_container<Traits>::find_type const & what) 
{	
	return object_ref.cont.find(what);
}

template <class Traits, class SafeCallback>
static inline bool container_safe_find(
	basic_safe_container<Traits> & object_ref, 
	typename basic_safe_container<Traits>::find_type const & what, 
	SafeCallback callback) 
{
	typename basic_safe_container<Traits>::iterator_type found; 
	typename basic_safe_container<Traits>::item_type item; 

	boost::lock_guard<boost::mutex> guard(object_ref.lock);
	found = container_find_unsafe<Traits>(object_ref, what);
	if (found != object_ref.cont.end()) { 
		callback(found);
		return true;
	}
	return false;
}

template <class Traits>
static inline bool container_safe_insert(
	basic_safe_container<Traits> & object_ref, 
	typename basic_safe_container<Traits>::find_type const & where, 
	typename basic_safe_container<Traits>::item_cref value) 
{
	boost::lock_guard<boost::mutex> guard(object_ref.lock);
	if (container_find_unsafe(object_ref, where) == 
		object_ref.cont.end()) 
	{
		object_ref.cont[where] = value;
		return true;
	}	
	return false;
}

template <class Traits>
static inline void container_safe_remove(
	basic_safe_container<Traits> & object_ref, 
	typename basic_safe_container<Traits>::find_type const & what) 
{
	typename basic_safe_container<Traits>::iterator_type where; 

	boost::lock_guard<boost::mutex> guard(object_ref.lock);
	where = container_find_unsafe(object_ref, what);
	if (where != object_ref.cont.end()) 
		object_ref.cont.erase(where);
}

template <class Traits, class SafeCallback>
static inline void container_safe_for_each(
	basic_safe_container<Traits> & object_ref, SafeCallback callback) 
{
	boost::lock_guard<boost::mutex> guard(object_ref.lock);
	std::for_each(object_ref.cont.begin(), object_ref.cont.end(), callback);
}

} // namespace utility

#endif

