#include "t2h_settings_manager.hpp"

#include <sstream>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define ADD_KEY_TYPE(x_)						\
struct key_##x_									\
	: public t2h_core_details::base_key 		\
{												\
	key_##x_() : base_key(#x_) { }				\
};												\

namespace t2h_core {

namespace t2h_core_details {

ADD_KEY_TYPE(workers)
ADD_KEY_TYPE(doc_root)
ADD_KEY_TYPE(server_addr)
ADD_KEY_TYPE(server_port)
ADD_KEY_TYPE(port_start)
ADD_KEY_TYPE(port_end)
ADD_KEY_TYPE(max_wait_time)

static inline void set_key(boost::property_tree::ptree & parser, 
			setting_manager::key_base_ptr key) 
{
	key->value = parser.get<std::string>(key->name);
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
	reset_configurations(); last_error_.clear();
	std::ifstream stream(conf_path.string().c_str());
	fill_config(stream);
}

void setting_manager::init_config(std::string const & json) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	std::istringstream json_stream(json);
	reset_configurations(); last_error_.clear();
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
	catch (boost::property_tree::json_parser::json_parser_error const & expr) 
	{
		last_error_ = expr.what();
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
	/** Add key to http server setting key-value factory */
	key_storage_->reg<key_workers>("workers");
	key_storage_->reg<key_doc_root>("doc_root");
	key_storage_->reg<key_server_addr>("server_addr");
	key_storage_->reg<key_server_port>("server_port");
	key_storage_->reg<key_port_start>("port_start");
	key_storage_->reg<key_port_end>("port_end");
	key_storage_->reg<key_max_wait_time>("max_wait_time");
}

} // namespace t2h_core

#undef ADD_KEY_TYPE

