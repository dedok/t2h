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

#if defined(WIN32)
#	pragma warning(push)
# 	pragma warning(disable : 4700) 
#endif

namespace t2h_core {

namespace t2h_core_details {

struct base_key {
	base_key(std::string const & name_, std::string const & default_value_) 
		: name(name_), value(), default_value(default_value_) 
	{ 
	}
	
	std::string const name;
	std::string value;
	std::string const default_value;
};

} // namespace t2h_core_details

class setting_manager_exception : public std::exception {
public :
	explicit setting_manager_exception(std::string const & message)  
		: message_(message) { }
	
	virtual ~setting_manager_exception() throw() { }

	virtual const char* what() const throw() 
	{ 
		return message_.c_str();
	}

private :
	std::string mutable message_;
};

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
		if (!key_storage_) 
			throw setting_manager_exception("setting manager was not init well"); 
		key_base_ptr key = key_storage_->get(name);
		if (key) { 
			return (!key->value.empty() ? 
				utility::safe_lexical_cast<T>(key->value) : panic_if_default_value_not_exists<T>(key));
		}
		return utility::default_ctor<T>();
	}

private :
	void init_keys_storage();
	void fill_config(std::istream & json_stream);
	void reset_configurations();
	
	template <class T>
	inline T panic_if_default_value_not_exists(key_base_ptr key) 
	{
		if (key->default_value.empty()) {
			throw setting_manager_exception(
				std::string("no default value for the key : " + key->name));
		}
		return utility::safe_lexical_cast<T>(key->default_value);
	}

	keys_factory_type * key_storage_;

	std::string last_error_;
	boost::mutex mutable lock_;
};

typedef setting_manager::ptr_type setting_manager_ptr;

} // namespace t2h_core

#if defined(WIN32)
#	pragma warning(pop)
#endif

#endif

