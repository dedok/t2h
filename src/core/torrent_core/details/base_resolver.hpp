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
	libtorrent::torrent_handle handle;
	int resolver_type;
	boost::posix_time::time_duration add_time;
};

inline bool operator==(resolve_info const & rhs, resolve_info const & lhs) 
{
	bool is_equal = false;
	LIBTORRENT_EXCEPTION_SAFE_BEGIN
	boost::filesystem::path full_path_first = rhs.handle.save_path(),
		full_path_second = lhs.handle.save_path();
	full_path_first = full_path_first / rhs.handle.name(); 
	full_path_second = full_path_second / lhs.handle.name();
	is_equal = (full_path_first == full_path_second);
	LIBTORRENT_EXCEPTION_SAFE_END
	return is_equal;
}

inline bool operator!=(resolve_info const & rhs, resolve_info const & lhs) 
{
	return !operator==(rhs, lhs);
}

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
	virtual ~base_resolver()  { }

	inline int get_type() const { return resolver_type_;  }
	virtual bool resolve(resolve_info & info) = 0;
	virtual bool is_need_resolve(resolve_info & info) = 0;

private :
	int const resolver_type_;

};

typedef base_resolver::ptr_type base_resolver_ptr;

int const default_resolver_type = std::numeric_limits<int>::min();

class default_resolver : public base_resolver {
public :
	default_resolver() : base_resolver(default_resolver_type) { }
	virtual bool resolve(resolve_info & info) { return true; }
	virtual bool is_need_resolve(resolve_info & info) { return false; }
	virtual ~default_resolver() { }
};

} } // namespace t2h_core, details 

#endif

