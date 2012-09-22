#ifndef ERROR_RESOLVERS_TYPES_HPP_INCLUDED
#define ERROR_RESOLVERS_TYPES_HPP_INCLUDED

#include "base_resolver.hpp"

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
		base_resolver(hang_downloading_type), 
		hang_duration_(boost::posix_time::seconds(10)) { }
	virtual ~hang_at_start_error() { }
	
	virtual bool resolve(resolve_info & info);
	virtual bool is_need_resolve(resolve_info & info);
	
private :
	boost::posix_time::time_duration const hang_duration_;

};

class connection_error : public base_resolver {
public :
	connection_error() : base_resolver(connection_error_type) { }
	virtual bool resolve(resolve_info & info);
	virtual bool is_need_resolve(resolve_info & info);
	virtual ~connection_error() { }
};

class unknown_error : public base_resolver {
public :
	unknown_error() : base_resolver(unknown_error_type) { }
	virtual bool resolve(resolve_info & info);
	virtual bool is_need_resolve(resolve_info & info);
	virtual ~unknown_error() { }
};

} } // namspace t2h_core, details

#endif

