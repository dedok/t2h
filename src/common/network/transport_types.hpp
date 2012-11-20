#ifndef TRANSPORT_TYPE_HPP_INCLUDED
#define TRANSPORT_TYPE_HPP_INCLUDED

#include "base_transport.hpp"
#include "http_transport_context.hpp"
#include "base_transport_ostream.hpp"
#include "http_mongoose_transport.hpp"

namespace common {
	/*  */
	typedef details::http_mongoose_transport http_mongoose_transport;

} // namespace common

#endif

