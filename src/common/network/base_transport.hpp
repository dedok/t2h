#ifndef BASE_TRANSPORT_HPP_INCLUDED
#define BASE_TRANSPORT_HPP_INCLUDED

#include "base_transport_ev_handler.hpp"

#include <exception>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace common { 

class transport_exception : public std::exception {
public :
	explicit transport_exception(std::string const & message);
	~transport_exception() throw();

	virtual char const * what() const throw();

private :
	std::string mutable message_;

};

struct transport_config {
	std::string ip_addr; 
	std::string port;
	std::size_t max_threads;
};

class base_transport : private boost::noncopyable {
public :
	typedef boost::shared_ptr<base_transport> ptr_type;
	base_transport(transport_config const & config, base_transport_ev_handler_ptr ev_handler);
	virtual ~base_transport();

	virtual void initialize() = 0;
	virtual void establish_connection() = 0;
	virtual bool is_connected() const = 0;
	virtual void stop_connection() = 0;
	virtual void wait() = 0;

	virtual void registr_event_handler(base_transport_ev_handler_ptr ev_handler) = 0;
	virtual base_transport_ev_handler_ptr get_event_handler() = 0;
	
private :

};

typedef base_transport::ptr_type base_transport_ptr;

} // namespace common

#endif

