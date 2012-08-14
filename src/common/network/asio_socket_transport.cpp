#include "asio_socket_transport.hpp"

#include <boost/bind.hpp>
#include <boost/exception/all.hpp>

namespace common { namespace details {

/**
 *	Public asio_socket_transport api
 */
asio_socket_transport::asio_socket_transport(
	transport_config const & config, 
	base_transport_ev_handler_ptr event_handler) 
		: base_transport(config, event_handler), 
		thread_pool_(),
		io_service_(), 
		signals_(io_service_),
		acceptor_(io_service_), 
		new_connection_(), 
		event_handler_(event_handler), 
		config_(config),
		well_configured_(false)
{ 
}

asio_socket_transport::~asio_socket_transport() 
{
}

void asio_socket_transport::initialize() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	
	if (is_connected_unsafe() && event_handler_) 
		throw (transport_exception(
			"This transport already started or event hander not valid"));

	try 
	{	
		setup_and_start_listen_acceptor();
		start_accept();
	} 
	catch (boost::exception const & expt) 
	{
		throw (transport_exception(boost::diagnostic_information(expt)));
	}
	
	well_configured_ = true;
}

void asio_socket_transport::establish_connection() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	if (!is_connected_unsafe() && !has_configured()) 
		throw (transport_exception("Transport alread started or not initialized"));
	
	try 
	{
/*		thread_pool_.resize_max_threads(config_.max_threads);
		for (std::size_t task_counter = 0; 
			task_counter < config_.max_threads; 
			++task_counter)
		{
			thread_pool_.add_task(task_counter,
				boost::bind(&boost::asio::io_service::run, &io_service_));	
		}
*/
		  std::vector<boost::shared_ptr<boost::thread> > threads;
		    for (std::size_t i = 0; i < 4; ++i)
			{
			      boost::shared_ptr<boost::thread> thread(new boost::thread(
				            boost::bind(&boost::asio::io_service::run, &io_service_)));
							    threads.push_back(thread);
			}

			for (std::size_t i = 0; i < threads.size(); ++i)
				threads[i]->join();
	}
	catch (boost::exception const & expt)
	{
		throw (transport_exception(boost::diagnostic_information(expt)));
	}
}

bool asio_socket_transport::is_connected() const 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	return is_connected_unsafe();
}

void asio_socket_transport::stop_connection() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	io_service_.stop();
	event_handler_->on_close();
	thread_pool_.wait_all_tasks();
}
		
void asio_socket_transport::registr_event_handler(base_transport_ev_handler_ptr event_handler) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	event_handler_ = event_handler;
}

base_transport_ev_handler_ptr asio_socket_transport::get_event_handler() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	return event_handler_;
}

/**
 * Private asio_socket_transport api
 */

bool asio_socket_transport::is_connected_unsafe() const 
{
	return thread_pool_.has_unfinished_task();
}

bool asio_socket_transport::has_configured() const 
{
	return well_configured_;
}

void asio_socket_transport::setup_and_start_listen_acceptor() 
{
	signals_.async_wait(boost::bind(&asio_socket_transport::handle_stop, this));
	
	// prepare ip v4/v6 addr resolver
	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::resolver::query query(config_.ip_addr, config_.port);
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	
	// bind addr and port, setup acceptor and start listen(without starting accept)
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
}

void asio_socket_transport::start_accept()
{
	connection * new_conn = new (std::nothrow) connection(io_service_, event_handler_);
	if (new_conn) {
		new_connection_.reset(new_conn);
		acceptor_.async_accept(new_connection_->socket(), 
			boost::bind(&asio_socket_transport::handle_accept, this,
			boost::asio::placeholders::error));
		return;
	} 
	event_handler_->on_error((int)base_transport_ev_handler::unknown_error);
}

void asio_socket_transport::handle_accept(boost::system::error_code const & error)
{
	(!error) ? new_connection_->start() : event_handler_->on_error(error.value());
	start_accept();
}

void asio_socket_transport::handle_stop()
{
	io_service_.stop();
	event_handler_->on_error(0); 
}

} } // namespace common, details
