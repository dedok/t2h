#ifndef HTTP_MONGOOSE_SOCKET_OSTREAM_HPP_INCLUDED
#define HTTP_MONGOOSE_SOCKET_OSTREAM_HPP_INCLUDED

#if defined(LC_USE_MONGOOSE_C_API)
extern "C" { 
#include "mongoose.h"
}
#else
#include "mongoose.h"
#endif // LC_MONGOOSE_C_API

#include "base_transport_ostream.hpp"

namespace common { namespace details {

/**
 *
 */
class mongoose_socket_ostream : public base_transport_ostream {
public :
	explicit mongoose_socket_ostream(struct mg_connection * conn);
	~mongoose_socket_ostream();
	
	virtual std::size_t write(char const * bytes, std::size_t bytes_size);
	virtual void async_write(
		char const * bytes, std::size_t bytes_size, write_compeletion_routine_type com_routine);

private :
	struct mg_connection * conn_;

};

} } // nemespace common, details 

#endif

