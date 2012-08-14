#ifndef BASE_TRANSPORT_EV_HANDLER_HPP_INCLUDED
#define BASE_TRANSPORT_EV_HANDLER_HPP_INCLUDED

#include <vector>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace common {

class base_transport_ev_handler 
	: private boost::noncopyable, 
	public boost::enable_shared_from_this<base_transport_ev_handler> 
{
public :
	enum known_error_state {
		user_interrupt = 0x1,
		unknown_error,
	};

	enum recv_result {
		more_data = 0x1,
		bad_data,
		sent_answ,
	};

	typedef char byte_type;
	typedef boost::shared_ptr<base_transport_ev_handler> ptr_type;
	typedef std::vector<char> buffer_type;

	base_transport_ev_handler();
	virtual ~base_transport_ev_handler();

	virtual recv_result on_recv(buffer_type const & recv_data, std::size_t recv_data_size, buffer_type & answ_data) = 0;
	virtual void on_close() = 0;
	virtual void on_error(int error_code) = 0;

	virtual ptr_type clone() = 0;
	
private :

};

typedef base_transport_ev_handler::ptr_type base_transport_ev_handler_ptr;

} // namespace common

#endif

