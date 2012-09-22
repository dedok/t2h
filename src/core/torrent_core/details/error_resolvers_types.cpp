#include "error_resolvers_types.hpp"

#include "torrent_core_macros.hpp"

namespace t2h_core { namespace details {

void hang_at_start_error::set_resolve_info(resolve_info const & info) 
{
	res_info_ = info;
}

bool hang_at_start_error::resolve() 
{
	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	if (!res_info_.status.handle.is_valid()) {
		TCORE_WARNING("Can not resolve hang_at_start_error, torrent handle not valid")
		return false;
	}
	
	if (utility::get_current_time() >= end_of_delay_) {
		TCORE_TRACE("Torrent error resolving: type 'hang downloading'", res_info_.status.handle.name().c_str())
		res_info_.status.handle.force_recheck();
		return true;
	}

	LIBTORRENT_EXCEPTION_SAFE_END
	
	return false;
}

bool hang_at_start_error::is_need_resolve(resolve_info & info) 
{	
	using libtorrent::torrent_status;

	LIBTORRENT_EXCEPTION_SAFE_BEGIN
		
	torrent_status & status = info.status;
	
	if (!status.paused && status.state == torrent_status::downloading && 
		(status.num_peers == 0 && status.connect_candidates != 0) &&
		status.progress == 0.0f)
	{
		TCORE_TRACE("Torrent error detected : type 'hang downloading' , torrent '%s'", info.status.handle.name().c_str())
		return true;
	}

	LIBTORRENT_EXCEPTION_SAFE_END_(return false)

	return false;
}

void connection_error::set_resolve_info(resolve_info const & info) 
{
	res_info_ = info;
}

bool connection_error::resolve() 
{
	return true;
}

bool connection_error::is_need_resolve(resolve_info & info) 
{
	return false;
}

void unknown_error::set_resolve_info(resolve_info const & info) 
{
	res_info_ = info;
}

bool unknown_error::resolve() 
{
	return true;
}

bool unknown_error::is_need_resolve(resolve_info & info) 
{
	return false;
}

} } // namspace t2h_core, details

