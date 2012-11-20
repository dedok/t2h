#ifndef HTTP_MONGOOSE_TRANSPORT_HPP_INCLUDED
#define HTTP_MONGOOSE_TRANSPORT_HPP_INCLUDED

#include "base_transport.hpp"
#include "http_transport_context.hpp"
#include "http_mongoose_socket_ostream.hpp"

#include <boost/thread.hpp>

#if defined(LC_USE_MONGOOSE_C_API)
extern "C" { 
#include "mongoose.h"
}
#else
#include "mongoose.h"
#endif // LC_MONGOOSE_C_API

namespace common { namespace details {

class http_mongoose_transport : public base_transport {
public :
	explicit http_mongoose_transport(transport_config const & config);
	virtual ~http_mongoose_transport();

	virtual void initialize();
	virtual void establish_connection();
	virtual bool is_connected() const;
	virtual void stop_connection();
	virtual void wait();

	void * dispatch_http_message(enum mg_event event, struct mg_connection * conn, struct mg_request_info const * ri);

private :
	void validate_config() const;

	boost::mutex mutable lock_;
	boost::condition_variable mutable waiter_;
	boost::mutex mutable waiter_lock_;
	struct mg_context * mg_handle_;
	transport_config mutable config_;
	http_transport_event_handler_ptr http_context_;
	bool stop_;

};

} } // namespace common, details 

#endif

