#ifndef ASIO_SOCKET_TRANSPORT_HPP_INCLUDED
#define ASIO_SOCKET_TRANSPORT_HPP_INCLUDED

#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "base_transport.hpp"
#include "connection.hpp"
#include "thread_pool.hpp"

namespace common { namespace details {

class asio_socket_transport : public base_transport {
public :
	asio_socket_transport(transport_config const & config, 
		base_transport_ev_handler_ptr event_handler);
	virtual ~asio_socket_transport();
	
	virtual void initialize();
	virtual void establish_connection();
	virtual bool is_connected() const;
	virtual void stop_connection();
		
	virtual void registr_event_handler(base_transport_ev_handler_ptr event_handler);
	virtual base_transport_ev_handler_ptr get_event_handler();
	
private :
	typedef utility::base_thread_pool<
		utility::details::default_policy
	> thread_pool_type;

	bool is_connected_unsafe() const;
	bool has_configured() const;

	void setup_and_start_listen_acceptor(); 

	void start_accept();
	void handle_accept(boost::system::error_code const & error);
	void handle_stop();

	thread_pool_type thread_pool_;
	boost::asio::io_service io_service_;
	boost::asio::signal_set signals_;
	boost::asio::ip::tcp::acceptor acceptor_;	
	connection_ptr new_connection_;	
	base_transport_ev_handler_ptr event_handler_;
	transport_config mutable config_;
	boost::mutex mutable lock_;
	bool well_configured_;
};

} } // namespace common, details

#endif 

