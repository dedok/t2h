#include "http_mongoose_transport.hpp"

#include "lc_trace.hpp"
#include "http_utility.hpp"
#include "http_reply_resources.hpp"

#include <boost/assert.hpp>

#define NOT_NULL (void *)1
#define STD_EXCEPTION_HANDLE_START try {
#define STD_EXCEPTION_HANDLE_END } catch (std::exception const & extp) { }

#define T2H_DEEP_DEBUG

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101)
#	pragma warning(disable : 4551)
#endif // WIN32

namespace common { namespace details {

/**
 * Hidden private http_mongoose_transport api
 */

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
		return utility::http_translate_range_header_c(rheader, range_header);
	return false;
}

static inline bool mon_is_head_request(struct mg_connection * conn, struct mg_request_info const * ri) 
{
	BOOST_ASSERT(ri != NULL);
	if ((strcmp(ri->http_version, "1.1") == 0 || (strcmp(ri->http_version, "1.0") == 0))) 
		if (strcmp(ri->request_method, "HEAD") == 0) 
			return true;
	return false;
}

static inline bool mon_is_content_requst(struct mg_connection * conn, struct mg_request_info const * ri) 
{
	BOOST_ASSERT(ri != NULL);
 	if ((strcmp(ri->http_version, "1.1") == 0 || strcmp(ri->http_version, "1.0") == 0))
		if ((strcmp(ri->request_method, "GET") == 0  || strcmp(ri->request_method, "POST") == 0))  
			return true;
	return false;
}

static void * redirect_to_http_mongoose_transport_dispatcher(enum mg_event event, struct mg_connection * conn) 
{
	/*  Redirect all events from mongoose to http_mongoose_transport object for handling by
	 	calling http_mongoose_transport::dispatch_http_message */
	struct mg_request_info const * ri = mg_get_request_info(conn);
	http_mongoose_transport * this_ = static_cast<http_mongoose_transport *>(ri->user_data);
	if (!this_) 
		return NULL;
	return (!this_->is_connected()) ? NULL : this_->dispatch_http_message(event, conn, ri);
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
	
	const char * mongoose_options[] = {
		"document_root", config_.doc_root.c_str(),		// doc root
		"listening_ports", config_.port.c_str(),		// avaliavle ports
		"enable_directory_listing", "no",				// off directory listing
		"num_threads", "6",								// number of wrokers
		NULL											// options end
	};

	mg_handle_ = mg_start(&redirect_to_http_mongoose_transport_dispatcher, 
						static_cast<void*>(this), mongoose_options);
	if (!mg_start) 
		throw transport_exception("can not start mongoose");

	if (!config_.context)
		throw transport_exception("http_*_transport::context not valid or ill configurated");
	
	http_context_ = 
			transport_context_cast<http_transport_event_handler>(config_.context);	
	
	stop_ = false;
}
	
bool http_mongoose_transport::is_connected() const 
{
	boost::lock_guard<boost::mutex> guard(lock_);	
	return (!stop_);
}
	
void http_mongoose_transport::stop_connection() 
{
	/*  Before we can drop wait function we must secure the mongoose stopped his work,
	 	otherwise UB */
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
	} // wait loop
}
	
void * http_mongoose_transport::dispatch_http_message(
	enum mg_event event, struct mg_connection * conn, struct mg_request_info const * ri) 
{
	/*  Because are we have not sure about each the abstract context call, 
	 	we must prevent any chance to lose exception from context functions */
	STD_EXCEPTION_HANDLE_START

	utility::range_header rheader;	
	base_transport_ostream_ptr socket_ostream(new details::mongoose_socket_ostream(conn));
	std::string const uri = utility::http_normalize_uri_c(ri->uri);
	switch (event) 
	{	
		/*  From the client came new request, first detect what type of the request, 
		 	second call subroutines for action. */
		case MG_NEW_REQUEST :
#if defined(T2H_DEEP_DEBUG)
			LC_TRACE("new request came from '%i', method '%s', uri '%s'", 
				ri->remote_ip, ri->request_method, ri->uri)
#endif // T2H_DEEP_DEBUG
			if (mon_is_range_request(conn, ri, rheader)) 
				http_context_->on_partial_content_request(socket_ostream, uri , rheader);
			if (mon_is_head_request(conn, ri))
				http_context_->on_head_request(socket_ostream, uri);
			if (mon_is_content_requst(conn, ri))
				http_context_->on_content_request(socket_ostream, uri);
#if defined(T2H_DEEP_DEBUG)
			LC_TRACE("reply was sended for '%i', method '%s', uri '%s'", 
				ri->remote_ip, ri->request_method, ri->uri)
#endif // T2H_DEEP_DEBUG
			return NOT_NULL;
		
		/* Was error on client request/reply */
		case MG_HTTP_ERROR :
			LC_WARNING("transport error for '%i', with uri '%s', with error message '%s'", 
				ri->remote_ip, ri->uri, (char *)ri->ev_data)
			return NULL;
		
		/* Reply was sended to client */
		case MG_REQUEST_COMPLETE :
#if defined(T2H_DEEP_DEBUG)
			LC_TRACE("reply was sended for '%i', method '%s', uri '%s'", 
				ri->remote_ip, ri->request_method, ri->uri)
#endif // T2H_DEEP_DEBUG
			return NULL ;
		
		case MG_EVENT_LOG :
			return NOT_NULL;

		default : 
			return NULL;
	} // switch
	
	STD_EXCEPTION_HANDLE_END
	LC_WARNING("unexpected exit")
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

