#ifndef MISC_UTILITY_HPP_INCLUDED
#define MISC_UTILITY_HPP_INCLUDED

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace utility {

template <class T>
inline T default_ctor() 
{
	T t;
	return t; 
}

template <class T, class U>
inline static T safe_lexical_cast(U const & u)
{
	try 
	{
		T const t =  boost::lexical_cast<T>(u);
		return t;
	} 
	catch (boost::bad_lexical_cast const &) 
	{ /**/ 
	}
	return default_ctor<T>();
}

std::string get_random_string(std::size_t dist_max = 8);

boost::posix_time::time_duration get_current_time();

} // namespace utility

#endif

