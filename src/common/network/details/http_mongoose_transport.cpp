#include "http_mongoose_transport.hpp"

#include "lc_trace.hpp"
#include "http_utility.hpp"
#include "http_reply_resources.hpp"

#include <boost/assert.hpp>

#define NOT_NULL (void *)1
#define STD_EXCEPTION_HANDLE_START try {
#define STD_EXCEPTION_HANDLE_END } catch (std::exception const & extp) { }

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101)
#	pragma warning(disable : 4551)
#endif // WIN32

namespace common { namespace details {

/**
 * Hidden private http_mongoose_transport api
 */

/* mongoose helpers */
static inline bool mon_is_range_request(
	struct mg_connection * conn, struct mg_request_info const * ri, utility::range_header & rheader) 
{
	BOOST_ASSERT(ri != NULL);

	if (strcmp(ri->http_version, "1.1") != 0)
		return false;
		
	if (strcmp(ri->request_method, "GET") != 0)
		return false;

	char const * range_header = mg_get_header(conn, "Range");
	if (range_header) 
		return utility::http_translate_range_header(rheader, range_header);
		
	char const * accept_header = mg_get_header(conn, "Accept"); 
	if (accept_header) 
		return utility::http_translate_accept_header(rheader, range_header); 
		
	return false;
}

static inline bool mon_is_head_request(struct mg_connection * conn, struct mg_request_info const * ri) 
{
	BOOST_ASSERT(ri != NULL);
	bool const s = ((strcmp(ri->http_version, "1.1") != 0 | (strcmp(ri->http_version, "1.0") != 0)) & 
		strcmp(ri->request_method, "HEAD") != 0);
	return s;
}

static inline bool mon_is_content_requst(struct mg_connection * conn, struct mg_request_info const * ri) 
{
	BOOST_ASSERT(ri != NULL);
	bool const s = ((strcmp(ri->http_version, "1.1") != 0 | strcmp(ri->http_version, "1.0") != 0) & 
		(strcmp(ri->request_method, "GET") != 0  | strcmp(ri->request_method, "POST") != 0));
	return s;
}

static void * mongoose_completion_routine(enum mg_event event, struct mg_connection * conn) 
{
	struct mg_request_info const * ri = mg_get_request_info(conn);
	http_mongoose_transport * this_ = static_cast<http_mongoose_transport *>(ri->user_data);
	if (!this_) 
		return NULL;
	return this_->dispatch_http_message(event, conn, ri);
}

/* mics helpers */
static void * http_stock_reply(
	struct mg_connection * conn, 
	http_transport_event_handler::operation_status status, 
	http_transport_event_handler_ptr far_handler,
	struct mg_request_info const * ri)	
{
	using namespace utility;

	std::string reply;
	std::string const uri = utility::http_normalize_uri_c(ri->uri);
	
	switch (status) {
		case http_transport_event_handler::io_error: case http_transport_event_handler::write_op_error :
			reply = stock_replies::cast_to_string(http_reply::internal_server_error);
		break;
		case http_transport_event_handler::not_found :
			reply = stock_replies::cast_to_string(http_reply::not_found);
		break;
	}

	mg_write(conn, reply.c_str(), reply.size());
	
	far_handler->error(status, uri.c_str());
	return NOT_NULL;
}

/* Far handler routines(aka http_transport_event_handler) */
static void * far_handler_on_range_request(
	http_transport_event_handler_ptr far_handler, 
	struct mg_connection * conn, 
	struct mg_request_info const * ri, 
	utility::range_header const & rheader) 
{
	BOOST_ASSERT(ri != NULL);
	
	http_transport_event_handler::http_data pcd;
	std::string const uri = utility::http_normalize_uri_c(ri->uri);

	far_handler->on_get_partial_content_headers(pcd, rheader.bstart_1, rheader.bend_1, uri.c_str());
	if (pcd.op_status != http_transport_event_handler::ok)  
		return http_stock_reply(conn, pcd.op_status, far_handler, ri);
	
	if (mg_write(conn, pcd.reply_header.c_str(), pcd.reply_header.size()) > 0) {
		bool has_data = true;
		pcd.seek_offset_pos = rheader.bstart_1;
		for (boost::int64_t bytes_writed = 0;;) 
		{
			has_data = far_handler->on_get_content_body(pcd, 
					rheader.bstart_1, 
					rheader.bend_1, 
					bytes_writed, 
					uri.c_str());
					
			if (pcd.op_status != http_transport_event_handler::ok) 
				return http_stock_reply(conn, pcd.op_status, far_handler, ri);
			
			if ((bytes_writed = mg_write(conn, &pcd.io_buffer.at(0), pcd.last_readed)) <= 0) {
				far_handler->error(http_transport_event_handler::write_op_error, uri.c_str());
				break;
			} // if
			
			if (!has_data) 
				break;
		} // read & send loop
		return NOT_NULL;
	}
	
	LC_WARNING("writing header data failed, for uri '%s'", uri.c_str())
	return NOT_NULL;
}

static void * far_handler_on_head_request(
	transport_context_ptr far_handler, struct mg_connection * conn, struct mg_request_info const * ri) 
{
	BOOST_ASSERT(ri != NULL);
	// TODO impl this
	return NULL;
}

static void * far_handler_on_content_request(
	transport_context_ptr far_handler, struct mg_connection * conn, struct mg_request_info const * ri)
{
	BOOST_ASSERT(ri != NULL);
	// TODO impl this
	return 	NULL;
}


/**
 * Public http_mongoose_transport api
 */

http_mongoose_transport::http_mongoose_transport(transport_config const & config) : 
		base_transport(config), 
		lock_(), 
		mg_handle_(NULL), 
		config_(config), 
		http_context_(),
		stop_(true)
{
	BOOST_ASSERT(config_.context != NULL);
}

http_mongoose_transport::~http_mongoose_transport() 
{
	stop_connection();	
}

void http_mongoose_transport::initialize() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	validate_config();
}
	
void http_mongoose_transport::establish_connection() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	if (mg_handle_)
		throw transport_exception("this transport already in use");
	
	const char * options[] = {
		"document_root", config_.doc_root.c_str(),
		"listening_ports", config_.port.c_str(),
		"enable_directory_listing", "no",
		"num_threads", "6",
		NULL
	};

	mg_handle_ = mg_start(&mongoose_completion_routine, static_cast<void*>(this), options);
	if (!mg_start)
		throw transport_exception("can not start mongoose");

	if (!config_.context)
		throw transport_exception("http_*_transport::context not valid or ill configurated");
	http_transport_event_handler_ptr far_ev_handler = 
			transport_context_cast<http_transport_event_handler>(config_.context);
	http_context_ = far_ev_handler;
	stop_ = false;
	BOOST_ASSERT(http_context_ != NULL);
}
	
bool http_mongoose_transport::is_connected() const 
{
	boost::lock_guard<boost::mutex> guard(lock_);	
	return (!stop_);
}
	
void http_mongoose_transport::stop_connection() 
{
	boost::lock_guard<boost::mutex> guard(lock_);	
	if (mg_handle_) {
		stop_ = true;
		mg_stop(mg_handle_); mg_handle_ = NULL;	
		waiter_.notify_all();
	}
}
	
void http_mongoose_transport::wait() 
{
	for (;;) {
		boost::unique_lock<boost::mutex> locker(waiter_lock_);
		waiter_.wait(locker);
		if (!mg_handle_) return;
	} // loop
}
	
void * http_mongoose_transport::dispatch_http_message(
	enum mg_event event, struct mg_connection * conn, struct mg_request_info const * ri) 
{
	STD_EXCEPTION_HANDLE_START
	
	utility::range_header rheader;
	
	if (!is_connected()) return NULL;
	
	switch (event) 
	{	
		case MG_NEW_REQUEST : 
			if (mon_is_range_request(conn, ri, rheader)) 
				return far_handler_on_range_request(http_context_, conn, ri, rheader);
			else if(mon_is_head_request(conn, ri))
				return far_handler_on_head_request(http_context_, conn, ri);
			else if (mon_is_content_requst(conn, ri))
				return far_handler_on_content_request(http_context_, conn, ri);
			break;
		
		case MG_HTTP_ERROR :
			return NULL;
			
		case MG_REQUEST_COMPLETE :
			return NULL;

		default : 
			break;
	} // switch
	
	STD_EXCEPTION_HANDLE_END
	return NULL;
}


/**
 * Private http_mongoose_transport api
 */

void http_mongoose_transport::validate_config() const 
{
	// TODO add port/host tests
}

} } // namespace common, details 

#if defined(WIN32)
#	pragma warning(pop)
#endif // WIN32

#undef NOT_NULL
#undef STD_EXCEPTION_HANDLE_START
#undef STD_EXCEPTION_HANDLE_END

