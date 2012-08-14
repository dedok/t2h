#ifndef CONNECTION_HPP_INCLUDED
#define CONNECTION_HPP_INCLUDED

#include "base_transport_ev_handler.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace common { namespace details {

class connection
	: public boost::enable_shared_from_this<connection>,
	private boost::noncopyable
{
public:
	connection(boost::asio::io_service & io_service, 
			base_transport_ev_handler_ptr ev_handler);
	~connection();

	boost::asio::ip::tcp::socket & socket();

	void start();

private:
	void handle_read(boost::system::error_code const & error, std::size_t bytes_transferred);
	void handle_write(boost::system::error_code const & error);

	boost::asio::io_service::strand strand_;
	boost::asio::ip::tcp::socket socket_;
	base_transport_ev_handler_ptr ev_handler_;
	base_transport_ev_handler::buffer_type recv_buffer_;
	base_transport_ev_handler::buffer_type reply_buffer_;

};

typedef boost::shared_ptr<connection> connection_ptr;

} } // namespace common, datails

#endif 

