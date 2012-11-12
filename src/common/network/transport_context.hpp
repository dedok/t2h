#ifndef TRANSPORT_CONTEXT_HPP_INCLUDED
#define TRANSPORT_CONTEXT_HPP_INCLUDED

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace common {

/**
 *
 */
class transport_context : boost::noncopyable {
public :
	explicit transport_context(int type_) : 
		type(type_) 
	{ }
	
	virtual ~transport_context() { }
	inline int get_context_type() const { return type; }

	int const type;
};

typedef boost::shared_ptr<transport_context> transport_context_ptr;

template <class T>
inline boost::shared_ptr<T> transport_context_cast(transport_context_ptr tc) 
{
	if (tc->type == T::context_type)
		return boost::static_pointer_cast<T>(tc);
	return boost::shared_ptr<T>();
}

} // common

#endif

