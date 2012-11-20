#ifndef HTTP_SERVER_CORE_HPP_INCLUDED
#define HTTP_SERVER_CORE_HPP_INCLUDED

#include "http_utility.hpp"
#include "base_service.hpp"
#include "base_transport.hpp"
#include "setting_manager.hpp"
#include "transport_types.hpp"
#include "http_server_ostream_policy.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace t2h_core {

namespace details {

/**
 *
 */
struct hsc_local_config {
	std::string doc_root;								// document root(doc_root + decoded_uri)
	bool chunked_ostream;								// on/off chunked ostream 
	boost::int64_t max_chunk_size;						// max readed offset per transport IO
	std::size_t cores_sync_timeout;						// filesystem cores timeout, in secs
};

} // namespace details

/**
 * http server for connect torrent_core to http server
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

	/* Over HTTP headers operations. Inherited for common::http_transport_event_handler */
	virtual void on_partial_content_request(
		common::base_transport_ostream_ptr ostream, std::string const & uri, utility::range_header const & range);
	virtual void on_head_request(common::base_transport_ostream_ptr ostream, std::string const & uri);
	virtual void on_content_request(common::base_transport_ostream_ptr ostream, std::string const & uri);

private :
	details::http_server_ostream_policy_ptr get_ostream_policy(common::base_transport_ostream_ptr tostream);

	common::base_transport_ptr transport_;
	setting_manager_ptr setting_manager_;
	common::base_service::service_state volatile mutable cur_state_;
	details::file_info_buffer_ptr file_info_buffer_;
	details::hsc_local_config local_config_;

};

typedef boost::shared_ptr<http_server_core> http_server_core_ptr;

} // namespace t2h_core

#endif

