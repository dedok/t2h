#include "http_server_core.hpp"

#include "http_utility.hpp"
#include "transport_types.hpp"
#include "http_server_core_config.hpp"

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

inline static void add_etag_date_headers(std::string & out, details::hc_file_info_ptr const fi) 
{
	std::string const gtm_time = utility::http_get_gmt_time_string(),
		etag = utility::http_etag(fi->file_size, std::time(NULL));
	if (!gtm_time.empty())
		out += "Date: " + gtm_time + "\r\n";
	if (!etag.empty())
		out += "Etag: " + etag + "\r\n";
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

void http_server_core::on_get_partial_content_headers(
		common::http_transport_event_handler::http_data & http_d,
		boost::int64_t bytes_start, 
		boost::int64_t bytes_end, 
		char const * uri) 
{
	/*	Create 206(Partial-Content) http header and send this header to client before body.
	 */	
	details::hc_file_info_ptr fi;
	std::string const req_path = local_config_.doc_root + uri;
	
	if (!(fi = file_info_buffer_->get_info(req_path))) {
		HCORE_WARNING("can not find path '%s' in buffer", req_path.c_str())
		http_d.op_status = common::http_transport_event_handler::not_found;
		return; 
	} // if
	// if client wish whole file the passed bytes_end == 0 
	if (bytes_end == 0) 
		bytes_end = fi->file_size - 1;
	
	boost::int64_t content_size = 1 * ((bytes_end - bytes_start) + 1);
	content_size = (content_size > fi->file_size ? fi->file_size : content_size);
	std::string const gmt_time = utility::http_get_gmt_time_string();
	
	if (!file_info_buffer_->wait_avaliable_bytes(
			fi, 
			1024*1000, 
			360)) 
	{
		HCORE_WARNING("sync with file system failed, timeout expired, for path '%s'", req_path.c_str())
		http_d.op_status = common::http_transport_event_handler::io_error;
		return;
	} // if	
	
	/*  NOTE : Prepare Etag, Date, Last-Modified headers. Must be in UTC, according to 
	 	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.*/
	http_d.reply_header = "HTTP/1.1 206 Partial-Content\r\n";
	add_etag_date_headers(http_d.reply_header, fi);
	http_d.reply_header +=	
		"Content-Type: application/octet-stream\r\n"
		"Accept-Ranges: bytes\r\n"	
		"Content-Range: ";
	http_d.reply_header += "bytes " + boost::lexical_cast<std::string>(bytes_start) + "-" +
		boost::lexical_cast<std::string>(bytes_end) + "/" +  boost::lexical_cast<std::string>(fi->file_size);
	http_d.reply_header += "\r\nContent-Length: " + boost::lexical_cast<std::string>(content_size);
	http_d.reply_header += "\r\n\r\n";
	
	http_d.op_status = common::http_transport_event_handler::ok;
}	
	
void http_server_core::on_get_content_headers(http_data & http_d, char const * uri) 
{

}

void http_server_core::on_get_head_headers(http_data & http_d, char const * uri) 
{
	/* Create http HEAD reply 
	 */
#if 0
	details::hc_file_info_ptr fi;
	std::string const req_path = local_config_.doc_root + uri;
	
	if (!(fi = file_info_buffer_->get_info(req_path))) {
		HCORE_WARNING("preparinh http header failed, for '%s', file not exist", req_path.c_str)
		http_d.op_status = common::http_transport_event_handler::not_found;
		return; 
	}
#endif
}
	
/* Operations with content data */
bool http_server_core::on_get_content_body(
		common::http_transport_event_handler::http_data & http_d, 
		boost::int64_t bytes_start, 
		boost::int64_t bytes_end, 
		boost::int64_t bytes_writed,
		char const * uri) 
{
	/*  First we must ensure what we have file info in buffer. 
	 	Because real file size could be less then bytes_end(at moment of call), second 
		what we do is sync with torrent_core via blocking call 'wait_avaliable_bytes'.
		If bytes are avaliable then just send block to clien otherwise error.*/
	namespace io = boost::iostreams;
	
	details::hc_file_info_ptr fi;
	std::string const req_path = local_config_.doc_root + uri;
	std::ios::openmode const open_mode = std::ios::in | std::ios::binary;
	
	http_d.last_readed = 0;
	
	if (cur_state_ == base_service::service_stoped) 
		return false;

	if (!(fi = file_info_buffer_->get_info(req_path))) {
		http_d.op_status = common::http_transport_event_handler::not_found;
		return false;
	} // if

	// if client wish whole file the bytes_end var will equal to 0 
	if (bytes_end == 0) 
		bytes_end = fi->file_size;	
	
	boost::int64_t read_offset = 1 * (bytes_end - bytes_start) + 1;
	if (1024*1000 <= read_offset)
		read_offset = 1024*1000;
	if (http_d.seek_offset_pos + read_offset > fi->file_size)
		read_offset = fi->file_size;

	if (!file_info_buffer_->wait_avaliable_bytes(
			fi, 
			http_d.seek_offset_pos + read_offset + 100, 
			360)) 
	{
		http_d.op_status = common::http_transport_event_handler::io_error;
		return false;
	} // if	
	
	bool has_next_data_block = true;
	io::file_descriptor_source file_handle(req_path, open_mode);
	if (file_handle.is_open()) {
		if (http_d.io_buffer.size() < read_offset)
			http_d.io_buffer.resize(read_offset + 1);
			
		if (http_d.seek_offset_pos >= bytes_end) { 
			has_next_data_block = false; 
			http_d.seek_offset_pos = 1 * (bytes_end - (1 * (bytes_start - http_d.seek_offset_pos))); 
		} // if 
			
		if (io::seek(file_handle, http_d.seek_offset_pos, BOOST_IOS::beg) < 0) 
			return false;
		
		if ((http_d.last_readed =
			io::read(file_handle, &http_d.io_buffer.at(0), read_offset)) <= 0) 
		{
			return false;
		} // if

		http_d.seek_offset_pos += read_offset;
		return has_next_data_block;
	} // if
	
	http_d.op_status = common::http_transport_event_handler::io_error;
	return false;
}	
	
void http_server_core::error(
	common::http_transport_event_handler::operation_status status, char const * uri) 
{
	HCORE_WARNING("got error from http transport, status '%i', uri '%s'", (int)status, uri);
	// TODO impl this
}

/**
 * Private http_server_core api
 */

} // namespace t2h_core

