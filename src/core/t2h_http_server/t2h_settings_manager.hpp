#ifndef T2H_SETTINGS_MANGER_HPP_INCLUDED
#define T2H_SETTINGS_MANGER_HPP_INCLUDED

#include "common_utility.hpp"

#include <string>
#include <istream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace t2h_core {

namespace t2h_core_details {

struct base_key {
	explicit base_key(std::string const name_) 
		: name(name_), value() 
	{ 
	}
	std::string const name;
	std::string value;
};

} // namespace t2h_core_details

class setting_manager : private boost::noncopyable {
public :
	typedef boost::shared_ptr<setting_manager> ptr_type;
	typedef utility::factory<t2h_core_details::base_key> keys_factory_type; 
	typedef keys_factory_type::base_ptr key_base_ptr;

	setting_manager();
	~setting_manager();

	static inline ptr_type shared_manager() 
	{
		static ptr_type manager(new setting_manager());
		return manager;
	}
	
	void load_config(boost::filesystem::path const & conf_path);
	void init_config(std::string const & json);

	std::string get_last_error() const;
	bool config_is_well();

	template <class T>
	inline T get_value(std::string const & name) 
	{
		boost::lock_guard<boost::mutex> guard(lock_);
		if (key_storage_) {
			key_base_ptr key = key_storage_->get(name);
			return (key ? utility::safe_lexical_cast<T>(key->value) : T());
		}
		return T();

	}

private :
	void init_keys_storage();
	void fill_config(std::istream & json_stream);
	void reset_configurations();
	
	keys_factory_type * key_storage_;

	std::string last_error_;
	boost::mutex mutable lock_;
};

typedef setting_manager::ptr_type setting_manager_ptr;

} // namespace t2h_core


#endif

