#include "lookup_error.hpp"
#include "resolvers_factory.hpp"
#include "torrent_core_macros.hpp"

namespace t2h_core { namespace details {  

lookup_error::lookup_error(torrent_ex_info_ptr ex_info, libtorrent::torrent_status const & status) 
	: ex_info_(ex_info) 
{
	bool state = false; base_resolver_ptr resolver;
	boost::tie(state, resolver) = need_resolving(status);
	if (state)  
		ex_info_->resolver = resolver;
}

lookup_error::~lookup_error() 
{
	try_resolve();
}

void lookup_error::try_resolve() 
{
	if (ex_info_->resolver) {
		if (ex_info_->resolver->resolve()) 
			ex_info_->resolver.reset();
	}
}

boost::tuple<bool, base_resolver_ptr> lookup_error::need_resolving(libtorrent::torrent_status const & status) 
{
	if (ex_info_->resolver) 
		return boost::make_tuple(false, base_resolver_ptr());
	base_resolver_ptr resolver = resolvers_factory::get()->get_resolver(status);
	return boost::make_tuple(resolver ? true : false, resolver);
}

} } // namespace t2h_core, details

