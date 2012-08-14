#include "connection.hpp"

#include <boost/bind.hpp>

namespace common { namespace details {

connection::connection(
	boost::asio::io_service & io_service, 
	base_transport_ev_handler_ptr ev_handler)
		: strand_(io_service), 
		socket_(io_service), 
		ev_handler_(ev_handler),
		recv_buffer_(),
		reply_buffer_()
{
	recv_buffer_.resize(8192);
}

connection::~connection() 
{ 
}

boost::asio::ip::tcp::socket & connection::socket()
{
	return socket_;
}

void connection::start()
{
	socket_.async_read_some(boost::asio::buffer(boost::asio::buffer(recv_buffer_)),
		strand_.wrap(
			boost::bind(&connection::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
}

void connection::handle_read(boost::system::error_code const & error, std::size_t bytes_transferred)
{
	/** If an error occurs then no new asynchronous operations are started. 
		This means that all shared_ptr references to the connection object will
		disappear and the object will be destroyed automatically after this
		handler returns. The connection class's destructor closes the socket.
	*/
	if (!error) {
		base_transport_ev_handler::recv_result const recv_result 
			= ev_handler_->on_recv(recv_buffer_, bytes_transferred, reply_buffer_);
		
		switch (recv_result) {
			case base_transport_ev_handler::bad_data : case base_transport_ev_handler::sent_answ :
				boost::asio::async_write(socket_, boost::asio::buffer(reply_buffer_),
				strand_.wrap(
					boost::bind(&connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error)));
				break;
			case base_transport_ev_handler::more_data :
				socket_.async_read_some(boost::asio::buffer(recv_buffer_),
				strand_.wrap(
					boost::bind(&connection::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
			break;
			default : break;
		}
		return;
	} // !if
	ev_handler_->on_error(error.value());
}

void connection::handle_write(boost::system::error_code const & error)
{
	/** No new asynchronous operations are started. This means that all shared_ptr
		references to the connection object will disappear and the object will be
		destroyed automatically after this handler returns. The connection class's
		destructor closes the socket.
	*/
	if (!error) {
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		ev_handler_->on_close();
		return;
	} // !if
	ev_handler_->on_error(error.value());
	ev_handler_->on_close();
}

} } // namespace common, details

#undef UNREAL_CASES

