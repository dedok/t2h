#ifndef BASE_RESOLVER_HPP_INCLUDED
#define BASE_RESOLVER_HPP_INCLUDED

#include <limits>

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#if defined(__GNUG__)
#	pragma GCC system_header
#endif

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include "torrent_core_macros.hpp"

namespace t2h_core { namespace details {

struct resolve_info { 
	libtorrent::torrent_status status;
};

class base_resolver {
public :
	typedef boost::shared_ptr<base_resolver> ptr_type;
	
	template <class ResolverType>	
	static inline ptr_type constructor() 
	{ 
		ResolverType * resolver_impl = new ResolverType();
		return ptr_type(resolver_impl); 
	}

	explicit base_resolver(int resolver_type) : resolver_type_(resolver_type) { }
	base_resolver(int resolver_type, resolve_info const & info) : resolver_type_(resolver_type) { }
	virtual ~base_resolver() { }

	virtual ptr_type clone(resolve_info const & info) = 0; 
	virtual void set_resolve_info(resolve_info const & info) = 0;
	
	inline int get_type() const 
		{ return resolver_type_;  }

	virtual bool resolve() = 0;
	virtual bool is_need_resolve(resolve_info & info) = 0;

private :
	int const resolver_type_;
	
};

typedef base_resolver::ptr_type base_resolver_ptr;

int const default_resolver_type = std::numeric_limits<int>::min();

class default_resolver : public base_resolver {
public :
	default_resolver() : base_resolver(default_resolver_type) { }	
	virtual ~default_resolver() { }

	virtual base_resolver_ptr clone(resolve_info const & info) 
		{ return base_resolver_ptr(new default_resolver()); };
	
	virtual void set_resolve_info(resolve_info const & info) 
		{ /*Do nothing*/ }

	virtual bool resolve() 
		{ return true; }
	
	virtual bool is_need_resolve(resolve_info & info) 
		{ return false; }
};

} } // namespace t2h_core, details 

#endif

