#include "http_server_core.hpp"

#include "replies_types.hpp"
#include "transport_types.hpp"
#include "http_server_macroses.hpp"
#include "hs_chunked_ostream_impl.hpp"

#include <ctime>

#include <boost/filesystem.hpp>
#include <boost/iostreams/operations.hpp>  
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/file_descriptor.hpp> 

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

namespace t2h_core {

/**
 * Privite hidden helpers
 */

inline static common::transport_config from_setting_manager(setting_manager_ptr setting_manager) 
{
	common::transport_config const config = { 
		setting_manager->get_value<std::string>("doc_root"),
		setting_manager->get_value<std::string>("server_addr"),
		setting_manager->get_value<std::string>("server_port"),
		setting_manager->get_value<std::size_t>("workers"),
		common::http_transport_event_handler_ptr()
	};
	return config;
}

inline static details::hsc_local_config hcs_from_setting_manager(setting_manager_ptr setting_manager) 
{
	details::hsc_local_config const hcsc = { 
		setting_manager->get_value<std::string>("doc_root"),
		true, 
		320,
		1024*1000
	};
	
	boost::system::error_code error;
	if (!boost::filesystem::exists(hcsc.doc_root, error) || 
		!boost::filesystem::is_directory(hcsc.doc_root, error)) 
	{
		throw common::transport_exception("invalid settings doc_root not exist or not directory");
	}
	
	return hcsc;
} 

/**
 * Public http_server_core api
 */

char const * http_server_core::http_core_service_name = "t2h_http_core";

http_server_core::http_server_core(setting_manager_ptr setting_manager) : 
	common::base_service(http_core_service_name),
	boost::enable_shared_from_this<http_server_core>(),
	transport_(),
	setting_manager_(setting_manager),
	cur_state_(),
	file_info_buffer_(),
	local_config_()
{ 
}  

http_server_core::~http_server_core() 
{
	stop_service();
}

bool http_server_core::launch_service() 
{
	try 
	{
		if (cur_state_ == base_service::service_running) 
			return false;

		local_config_ = hcs_from_setting_manager(setting_manager_);
		common::transport_config tr_config = from_setting_manager(setting_manager_);
		tr_config.context = http_server_core::shared_this();
		
		file_info_buffer_ = details::shared_file_info_buffer();
		BOOST_ASSERT(file_info_buffer_ != NULL);

		transport_.reset(new common::http_mongoose_transport(tr_config));
		BOOST_ASSERT(transport_ != NULL);
		
		transport_->initialize();	
		transport_->establish_connection();
	
		cur_state_ = base_service::service_running;
	} 
	catch (common::transport_exception const & expt) 
	{
		HCORE_ERROR("transport init/run failed, with message '%s'", expt.what())
		return false;
	}
	return (cur_state_ == base_service::service_running);
}

void http_server_core::stop_service() 
{
	try 
	{
		if (cur_state_ == base_service::service_running) {
			cur_state_ = base_service::service_stoped;
			transport_->stop_connection();	
		}
	}
	catch (common::transport_exception const & expt) 
	{
		HCORE_ERROR("transport stop failed, with message '%s'", expt.what())
	}
}

void http_server_core::wait_service() 
{
	try 
	{
		if (cur_state_ == base_service::service_running)
			transport_->wait();
	} 
	catch (common::transport_exception const & expt) 
	{
		HCORE_ERROR("transport wait failed, with message '%s'", expt.what())
	}
}

common::base_service_ptr http_server_core::clone() 
{
	return boost::shared_ptr<http_server_core>(
		new http_server_core(setting_manager_));
}

common::base_service::service_state http_server_core::get_service_state() const 
{
	return cur_state_;
}

/**
 * Inherited http_server_core api
 */

void http_server_core::on_partial_content_request(
	common::base_transport_ostream_ptr ostream, std::string const & uri, utility::range_header const & range) 
{	
	boost::int64_t start = range.bstart_1, end = range.bend_1;
	std::string const req_path = local_config_.doc_root + uri;
	details::hc_file_info_ptr fi = file_info_buffer_->get_info(req_path);
	
	if (!fi) {
		HCORE_WARNING("can not find path '%s' in buffer", req_path.c_str())
		return;
	}
	
	if (range.bstart_1 == utility::range_header::all)
		start = 0;
	if (range.bend_1 == utility::range_header::all)
		end = fi->file_size;

	details::partial_content_reply pcr;
	details::http_data hdata = { fi, file_info_buffer_, start, end };
	details::http_server_ostream_policy_ptr ostream_policy = get_ostream_policy(ostream);
	if (!ostream_policy->perform(pcr, hdata))
		HCORE_WARNING("send partial content to the client failed")
}

void http_server_core::on_head_request(common::base_transport_ostream_ptr ostream, std::string const & uri) 
{
	std::string const req_path = local_config_.doc_root + uri;
	details::hc_file_info_ptr fi = file_info_buffer_->get_info(req_path);

	if (!fi) {
		HCORE_WARNING("can not find path '%s' in buffer", req_path.c_str())
		return;
	}
	
	details::head_reply hr;
	details::http_data hdata = { fi, file_info_buffer_, 0, fi->file_size };
	details::http_server_ostream_policy_ptr ostream_policy = get_ostream_policy(ostream);	
	if (!ostream_policy->perform(hr, hdata))
		HCORE_WARNING("send reply to the client failed")
}

void http_server_core::on_content_request(common::base_transport_ostream_ptr ostream, std::string const & uri) 
{
	std::string const req_path = local_config_.doc_root + uri;
	details::hc_file_info_ptr fi = file_info_buffer_->get_info(req_path);

	if (!fi) {
		HCORE_WARNING("can not find path '%s' in buffer", req_path.c_str())
		return;
	}
	
	details::send_content_reply scr;
	details::http_data hdata = { fi, file_info_buffer_, 0, fi->file_size };
	details::http_server_ostream_policy_ptr ostream_policy = get_ostream_policy(ostream);
	if (!ostream_policy->perform(scr, hdata))
		HCORE_WARNING("send reply to the client failed")
}
	
/**
 * Private http_server_core api
 */

details::http_server_ostream_policy_ptr http_server_core::get_ostream_policy(common::base_transport_ostream_ptr tostream) 
{
	details::http_server_ostream_policy_ptr ostream_policy;
	if (local_config_.chunked_ostream) {
		details::hs_chunked_ostream_params hcsp = 
			{ local_config_.max_chunk_size, local_config_.cores_sync_timeout };
		details::http_server_ostream_policy_params hsopp = { true };
		ostream_policy.reset(new details::hs_chunked_ostream_impl(hsopp, hcsp));
		ostream_policy->set_ostream(tostream);
		BOOST_ASSERT(ostream_policy != NULL);
	}
	return ostream_policy;
}

} // namespace t2h_core

