#include "setting_manager.hpp"

#include <sstream>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101) 
# 	pragma warning(disable : 4700) 
#endif

#define ADD_KEY_TYPE(x_, y_, r_, s_)			\
struct key_##x_									\
	: public t2h_core_details::base_key 		\
{												\
	key_##x_() : base_key(#x_, y_, r_, s_) { }	\
};												\

namespace t2h_core {

namespace t2h_core_details {

// http server keys, with some defaults values
ADD_KEY_TYPE(workers, "4", "", false)
ADD_KEY_TYPE(doc_root, "", "", true)
ADD_KEY_TYPE(server_addr, "", "", true)
ADD_KEY_TYPE(server_port, "80", "", false)
ADD_KEY_TYPE(tc_port_start, "", "", true)
ADD_KEY_TYPE(tc_port_end, "", "", true)

// torrent core keys, with some defaults values
ADD_KEY_TYPE(cores_sync_timeout, "360", "", false)
ADD_KEY_TYPE(hc_chunked, "true", "", false)
ADD_KEY_TYPE(hc_max_chunk_size, "102400", "", false)
ADD_KEY_TYPE(tc_max_alert_wait_time, "10", "", false)
ADD_KEY_TYPE(tc_max_partial_download_size, "2242880", "", false)
ADD_KEY_TYPE(tc_root, "", "doc_root", false)
ADD_KEY_TYPE(tc_futures_timeout, "10", "", false)
ADD_KEY_TYPE(tc_partial_files_download, "true", "", false)
ADD_KEY_TYPE(tc_max_connections_per_torrent, "50", "", false)
ADD_KEY_TYPE(tc_max_uploads, "-1", "", false)
ADD_KEY_TYPE(tc_upload_limit, "0", "", false)
ADD_KEY_TYPE(tc_download_limit, "0", "", false)
ADD_KEY_TYPE(tc_sequential_download, "true", "", false)
ADD_KEY_TYPE(tc_resolve_countries, "true", "", false)
ADD_KEY_TYPE(tc_resolve_checkout, "6", "", false)
ADD_KEY_TYPE(tc_auto_error_resolving, "true", "", false)
ADD_KEY_TYPE(tc_loadable_session, "true", "", false)

static inline void set_key(boost::property_tree::ptree & parser, 
			setting_manager::key_base_ptr key) 
{
	try 
	{
		key->value = parser.get<std::string>(key->name);
	} 
	catch (boost::property_tree::json_parser_error const & expt) 
	{
		if (key->req && key->default_value.empty())
			throw boost::property_tree::json_parser_error(expt);
		key->value.clear();
	}
	catch (std::exception const & expt) 
	{
		key->value.clear();
	}
}

} // namespace t2h_core_details

/**
 * Public setting_manager api
 */

setting_manager::setting_manager() 
	: key_storage_(NULL), last_error_(), lock_()
{
}

setting_manager::~setting_manager() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	delete key_storage_;
}

void setting_manager::load_config(boost::filesystem::path const & conf_path) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	boost::system::error_code error_code;
	reset_configurations(); last_error_.clear();
	if (!boost::filesystem::exists(conf_path, error_code)) { 
		last_error_ = "File at path '" + conf_path.string() + "' not exist";
		return;
	}
	std::ifstream stream(conf_path.string().c_str());
	fill_config(stream);
}

void setting_manager::init_config(std::string const & json) 
{
	boost::lock_guard<boost::mutex> guard(lock_);	
	reset_configurations(); last_error_.clear();
	if (json.empty()) {
		last_error_ = "bad json passed";
		return;
	}
	std::istringstream json_stream(json);
	fill_config(json_stream);
}

std::string setting_manager::get_last_error() const 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	return last_error_;
}

bool setting_manager::config_is_well() 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	return last_error_.empty();
}

/**
 * Private setting_manager api
 */

void setting_manager::fill_config(std::istream & json_stream) 
{
	try 
	{
		boost::property_tree::ptree ptree_parser;
		boost::property_tree::read_json(json_stream, ptree_parser);
		key_storage_->for_each(
			boost::bind(&t2h_core_details::set_key, 
				boost::ref(ptree_parser), 
				_1)
			);
	}
	catch (boost::property_tree::json_parser_error const & expt) 
	{
		last_error_ = expt.what();
	}
}

void setting_manager::reset_configurations() 
{
	delete key_storage_;
	key_storage_ = new keys_factory_type();
	init_keys_storage();
}

void setting_manager::init_keys_storage() 
{
	using namespace t2h_core_details;
	// http server settings
	key_storage_->reg<key_workers>("workers");
	key_storage_->reg<key_doc_root>("doc_root");
	key_storage_->reg<key_server_addr>("server_addr");
	key_storage_->reg<key_server_port>("server_port");
	key_storage_->reg<key_cores_sync_timeout>("cores_sync_timeout");
	key_storage_->reg<key_hc_max_chunk_size>("hc_max_chunk_size");
	key_storage_->reg<key_hc_chunked>("hc_chunked");

	// torrent core settings 
	key_storage_->reg<key_tc_port_start>("tc_port_start");
	key_storage_->reg<key_tc_port_end>("tc_port_end");
	key_storage_->reg<key_tc_max_alert_wait_time>("tc_max_alert_wait_time");
	key_storage_->reg<key_tc_max_partial_download_size>("tc_max_partial_download_size");
	key_storage_->reg<key_tc_root>("tc_root");
	key_storage_->reg<key_tc_futures_timeout>("tc_futures_timeout");
	key_storage_->reg<key_tc_partial_files_download>("tc_partial_files_download");
	key_storage_->reg<key_tc_max_connections_per_torrent>("tc_max_connections_per_torrent");
	key_storage_->reg<key_tc_max_uploads>("tc_max_uploads");
	key_storage_->reg<key_tc_upload_limit>("tc_upload_limit");
	key_storage_->reg<key_tc_download_limit>("tc_download_limit");
	key_storage_->reg<key_tc_sequential_download>("tc_sequential_download");
	key_storage_->reg<key_tc_resolve_countries>("tc_resolve_countries");
	key_storage_->reg<key_tc_resolve_checkout>("tc_resolve_checkout");
	key_storage_->reg<key_tc_auto_error_resolving>("tc_auto_error_resolving");
	key_storage_->reg<key_tc_loadable_session>("tc_loadable_session");
}

} // namespace t2h_core

#undef ADD_KEY_TYPE

#if defined(WIN32)
#	pragma warning(pop)
#endif
