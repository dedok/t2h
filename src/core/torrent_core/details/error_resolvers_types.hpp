#ifndef ERROR_RESOLVERS_TYPES_HPP_INCLUDED
#define ERROR_RESOLVERS_TYPES_HPP_INCLUDED

#include "base_resolver.hpp"
#include "misc_utility.hpp"

namespace t2h_core { namespace details {

enum error_resolvers_types {
	hang_downloading_type = 0x0,
	connection_error_type,
	no_error_detected,
	unknown_error_type = no_error_detected + 0x1
};

class hang_at_start_error : public base_resolver {
public :
	hang_at_start_error() : 
		base_resolver(hang_downloading_type) { }
	
	explicit hang_at_start_error(resolve_info const & info) 
		: base_resolver(hang_downloading_type, info), res_info_(info), 
			add_time_(utility::get_current_time()), end_of_delay_()
	{ 
		end_of_delay_ = add_time_ + boost::posix_time::seconds(5);
	}
	
	virtual ~hang_at_start_error() { }

	virtual base_resolver_ptr clone(resolve_info const & info) 
		{ return base_resolver_ptr(new hang_at_start_error(info)); }

	virtual void set_resolve_info(resolve_info const & info);

	virtual bool resolve();
	virtual bool is_need_resolve(resolve_info & info);
	
private :
	resolve_info res_info_;
	boost::posix_time::time_duration add_time_;	
	boost::posix_time::time_duration end_of_delay_;

};

class connection_error : public base_resolver {
public :
	connection_error() : base_resolver(connection_error_type) { }
	
	explicit connection_error(resolve_info const & info) 
		: base_resolver(connection_error_type, info), res_info_(info) { }

	virtual ~connection_error() { }
	
	virtual base_resolver_ptr clone(resolve_info const & info) 
		{ return base_resolver_ptr(new connection_error(info)); }

	virtual void set_resolve_info(resolve_info const & info);

	virtual bool resolve();
	virtual bool is_need_resolve(resolve_info & info);

private :
	resolve_info res_info_;

};

class unknown_error : public base_resolver {
public :
	unknown_error() : base_resolver(unknown_error_type) { }
	
	explicit unknown_error(resolve_info const & info) 
		: base_resolver(unknown_error_type, info), res_info_(info) { }	
	
	virtual ~unknown_error() { }
	
	virtual bool resolve();
	virtual bool is_need_resolve(resolve_info & info);

	virtual base_resolver_ptr clone(resolve_info const & info) 
		{ return base_resolver_ptr(new unknown_error(info)); }

	virtual void set_resolve_info(resolve_info const & info);

private :
	resolve_info res_info_;

};

} } // namspace t2h_core, details

#endif

