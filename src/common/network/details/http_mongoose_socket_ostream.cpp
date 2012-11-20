#include "http_mongoose_socket_ostream.hpp"

namespace common { namespace details {

mongoose_socket_ostream::mongoose_socket_ostream(struct mg_connection * conn) 
	: base_transport_ostream(), conn_(conn) 
{ 

}

mongoose_socket_ostream::~mongoose_socket_ostream() 
{ 

}
	
std::size_t mongoose_socket_ostream::write(char const * bytes, std::size_t bytes_size) 
{
	BOOST_ASSERT(conn_ != NULL);
	int writed = 0;
	if ((writed = mg_write(conn_, bytes, bytes_size)) <= 0)
		return 0;
	return writed;
}

void mongoose_socket_ostream::async_write(
		char const * bytes, std::size_t bytes_size, write_compeletion_routine_type com_routine) 
{
	BOOST_ASSERT(conn_ != NULL);
	std::size_t writed = mg_write(conn_, bytes, bytes_size);
	com_routine(-1, 0);
}

} } // nemespace common, details 

