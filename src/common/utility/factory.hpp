#ifndef UTILITY_FATORY_HPP_INCLUDED
#define UTILITY_FATORY_HPP_INCLUDED

#include <string>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace utility {

template <class Base>
class factory {
public:
    typedef boost::shared_ptr<Base> base_ptr;
   	
	factory() 
	{ 
	}
	
	~factory() 
	{ 
	}
	
	template <class Derived>
	inline void reg(std::string const & name) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		fmt_const_iterator found = factories_.find(name);
		if (found == factories_.end()) {
			base_type_ptr ptr(new derived_type<Derived>());
			factories_[name] = ptr;
		}
	}

	inline void unreg(std::string const & name) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		fmt_const_iterator found = factories_.find(name);
		if (found != factories_.end())
			factories_.erase(found);
	}
	
	inline base_ptr get(std::string const & name) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		fmt_iterator found = factories_.find(name);	
		if (found != factories_.end()) { 
			return factories_[name]->get();
		}
		return base_ptr();
	}

	template <class Func>
	void for_each(Func func) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		fmt_iterator it = factories_.begin(), end = factories_.end();
		for (; it != end; ++it)  
			func(it->second->get());
	}
	
private:
   	/**
	 * Details
	 */
	class base_type {
	public:
		virtual ~base_type() { }
		virtual base_ptr get() = 0;
	};
    
	typedef boost::shared_ptr<base_type> base_type_ptr;
    
	template <class T>
	class derived_type : public base_type {
	public:
		derived_type() : object_(new T()) { }

		virtual base_ptr get() 
		{
			return object_;
		}

	private :
		base_ptr object_;
	};
   	
	typedef boost::unordered_map<std::string, base_type_ptr> factory_map_type;
    typedef typename factory_map_type::iterator fmt_iterator;
	typedef typename factory_map_type::const_iterator fmt_const_iterator;
	
	factory_map_type factories_;
	boost::mutex mutable lock_;

};

} // namespace utility


#endif

