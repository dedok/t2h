#ifndef BASE_TRANSPORT_OSTREAM_HPP_INCLUDED
#define BASE_TRANSPORT_OSTREAM_HPP_INCLUDED

#include <cstddef>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace common {

/**
 *
 */
class base_transport_ostream {
public :
	typedef boost::function<void (int, std::size_t)> write_compeletion_routine_type;

	base_transport_ostream() { }
	virtual ~base_transport_ostream() { }

	virtual std::size_t write(char const * bytes, std::size_t bytes_size) = 0;
	virtual void async_write(
		char const * bytes, std::size_t bytes_size, write_compeletion_routine_type com_routine) = 0;
	
};

typedef boost::shared_ptr<base_transport_ostream> base_transport_ostream_ptr;

} // namespace common

#endif // BASE_SOCKET_OSTREAM_HPP_INCLUDED

