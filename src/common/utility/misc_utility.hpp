#ifndef MISC_UTILITY_HPP_INCLUDED
#define MISC_UTILITY_HPP_INCLUDED

#include <boost/lexical_cast.hpp>
#include <iostream>

namespace utility {

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
	return T();
}

} // namespace utility

#endif

