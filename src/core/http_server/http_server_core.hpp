#ifndef HTTP_SERVER_CORE_HPP_INCLUDED
#define HTTP_SERVER_CORE_HPP_INCLUDED

#include "base_service.hpp"
#include "base_transport.hpp"
#include "setting_manager.hpp"
#include "transport_types.hpp"
#include "file_info_buffer.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

/**
 *
 */
struct hsc_local_config {
	std::string doc_root;								// document root(doc_root + decoded_uri)
	std::size_t fs_cores_sync_timeout;					// filesystem cores timeout, in secs 
	boost::int64_t max_read_offset;						// max readed offset per transport IO
};

} // namespace details

/**
 *
 */
class http_server_core : 
	public common::base_service, 
	public common::http_transport_event_handler,
	public boost::enable_shared_from_this<http_server_core>
{
public :
	static char const * http_core_service_name;
	
	explicit http_server_core(setting_manager_ptr setting_manager);
	~http_server_core();
		
	virtual bool launch_service();
	virtual void stop_service();
	virtual void wait_service();
	
	virtual common::base_service::ptr_type clone();

	virtual common::base_service::service_state get_service_state() const;
	
 	inline boost::shared_ptr<http_server_core> shared_this()
	 { return shared_from_this(); }

	/* Over HTTP headers operations */
	virtual void on_get_partial_content_headers(
		common::http_transport_event_handler::http_data & http_d, 
		boost::int64_t bytes_start, 
		boost::int64_t bytes_end, 
		char const * uri);	
	virtual void on_get_content_headers(common::http_transport_event_handler::http_data & http_d, char const * uri);
	virtual void on_get_head_headers(common::http_transport_event_handler::http_data & http_d, char const * uri);
	
	/* Operations with content data */
	virtual bool on_get_content_body(
			common::http_transport_event_handler::http_data & http_d, 
			boost::int64_t bytes_start, 
			boost::int64_t bytes_end, 
			boost::int64_t bytes_writed,
			char const * uri);	
	
	/* Informers/Heplers */
	virtual void error(common::http_transport_event_handler::operation_status status, char const * uri);

private :
	common::base_transport_ptr transport_;
	setting_manager_ptr setting_manager_;
	common::base_service::service_state volatile mutable cur_state_;
	details::file_info_buffer_ptr file_info_buffer_;
	details::hsc_local_config local_config_;

};

typedef boost::shared_ptr<http_server_core> http_server_core_ptr;

} // namespace t2h_core

#endif

