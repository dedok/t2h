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
struct cast_adapter {
	inline static T safe_lexical_cast_impl(U const & u)
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
};

template <>
struct cast_adapter<bool, std::string> {
	inline static bool safe_lexical_cast_impl(std::string const & str) 
	{
		return (str == "true") ? true : false;
	}
};

template <class T, class U>
inline static T safe_lexical_cast(U const & u) 
{
	return cast_adapter<T, U>::safe_lexical_cast_impl(u); 
}

std::string get_random_string(std::size_t dist_max = 8);

boost::posix_time::time_duration get_current_time();

} // namespace utility

#endif

