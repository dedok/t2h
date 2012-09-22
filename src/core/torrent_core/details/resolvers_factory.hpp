#ifndef RESOLVERS_FACTORY_HPP_INCLUDED
#define RESOLVERS_FACTORY_HPP_INCLUDED

#include "base_resolver.hpp"
#include "torrent_info.hpp"

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace t2h_core { namespace details {

class resolvers_factory : boost::noncopyable {
public :
	typedef boost::shared_ptr<resolvers_factory> ptr_type;
	typedef std::map<int, base_resolver_ptr> resolvers_type;
	
	resolvers_factory();
	~resolvers_factory();
	
	static inline ptr_type get() 
	{
		static ptr_type this_(new resolvers_factory());
		return this_;
	}

	void add_resolver(base_resolver_ptr resolver);
	void remove_resolver(base_resolver_ptr resolver);

	base_resolver_ptr get_resolver(libtorrent::torrent_status const & status);	
	base_resolver_ptr get_resolver(int resolver_type, libtorrent::torrent_status const & status);

private :
	void register_default_resolvers();
	resolvers_type resolvers_;

};

typedef resolvers_factory::ptr_type resolvers_factory_ptr;

} } // namespace t2h_core, details 

#endif

