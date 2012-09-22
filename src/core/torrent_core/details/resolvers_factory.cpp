#include "resolvers_factory.hpp"

#include "misc_utility.hpp"
#include "error_resolvers_types.hpp"

#define ADD_DEFAULT_RESOLVER(res_list, x) 						\
do {															\
	base_resolver_ptr impl = base_resolver::constructor<x>();	\
	res_list[impl->get_type()] = impl;							\
} while (0);

namespace t2h_core { namespace details {

static inline resolve_info from_torrent_status(libtorrent::torrent_status const & status) 
{
	resolve_info info;
	info.status = status;
	return info;
}

/**
 * Public resolvers api 
 */
resolvers_factory::resolvers_factory() : resolvers_() 
{ 
	register_default_resolvers();
}

resolvers_factory::~resolvers_factory() 
{ 
}

void resolvers_factory::add_resolver(base_resolver_ptr resolver) 
{
	if (!resolver)
		return;

	if (resolvers_.count(resolver->get_type()) == 0)
		return;
	
	resolvers_[resolver->get_type()] = resolver;
}

void resolvers_factory::remove_resolver(base_resolver_ptr resolver) 
{
	if (!resolver)
		return;
	
	resolvers_type::iterator found = resolvers_.find(resolver->get_type());
	if (found != resolvers_.end())
		resolvers_.erase(found);
}

base_resolver_ptr resolvers_factory::get_resolver(int resolver_type, libtorrent::torrent_status const & status) 
{
	resolve_info info = from_torrent_status(status);
	if (resolvers_.count(resolver_type) != 0) {
		base_resolver_ptr resolver = resolvers_[resolver_type];
		if (resolver->is_need_resolve(info))
			return resolvers_[resolver_type]->clone(info);
	}
	return base_resolver_ptr();
}

base_resolver_ptr resolvers_factory::get_resolver(libtorrent::torrent_status const & status) 
{
	resolve_info info = from_torrent_status(status);
	for (resolvers_type::iterator it = resolvers_.begin(), last = resolvers_.end(); 
		it != last; 
		++it) 
	{
		if (it->second->is_need_resolve(info)) { 
			base_resolver_ptr resolver = it->second->clone(info);
			resolver->set_resolve_info(info);
			return resolver;
		}
	}
	return base_resolver_ptr();
}

/**
 * Private resolvers api
 */
void resolvers_factory::register_default_resolvers() 
{
	ADD_DEFAULT_RESOLVER(resolvers_, hang_at_start_error)
	ADD_DEFAULT_RESOLVER(resolvers_, connection_error)
	ADD_DEFAULT_RESOLVER(resolvers_, unknown_error)
}


} } // namespace t2h_core, details 

#undef ADD_DEFAULT_RESOLVER
