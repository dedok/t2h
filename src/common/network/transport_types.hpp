#ifndef TRANSPORT_TYPE_HPP_INCLUDED
#define TRANSPORT_TYPE_HPP_INCLUDED

#include "base_transport.hpp"            
#include "asio_socket_transport.hpp"

namespace common {
	typedef details::asio_socket_transport asio_socket_transport;
} // namespace common

#endif

