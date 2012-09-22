#ifndef LOOKUP_ERROR_HPP_INCLUDED 
#define LOOKUP_ERROR_HPP_INCLUDED

#include "torrent_info.hpp"
#include "base_resolver.hpp"

namespace t2h_core { namespace details {

class lookup_error {
public :
	lookup_error(torrent_ex_info_ptr ex_info, libtorrent::torrent_status const & status);
	~lookup_error();

private :
	boost::tuple<bool, base_resolver_ptr> 
		need_resolving(libtorrent::torrent_status const & status);
	void try_resolve();

	torrent_ex_info_ptr ex_info_;

};

} } // namespace t2h_core, details

#endif

