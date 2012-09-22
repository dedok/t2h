#include "error_resolvers_types.hpp"

#include "misc_utility.hpp"
#include "torrent_core_macros.hpp"

namespace t2h_core { namespace details {

bool hang_at_start_error::resolve(resolve_info & info) 
{
	LIBTORRENT_EXCEPTION_SAFE_BEGIN

	// check type, just for sure
	if (info.resolver_type == hang_downloading_type) {
		TCORE_TRACE("Torrent error resolving: type 'hang downloading'", info.handle.name().c_str())
		info.handle.force_recheck();
		info.add_time = utility::get_current_time();
		info.resolver_type = no_error_detected;
		return true;
	}

	LIBTORRENT_EXCEPTION_SAFE_END
	
	return false;
}

bool hang_at_start_error::is_need_resolve(resolve_info & info) 
{
	
	using libtorrent::torrent_status;

	bool is_problem = false;
	
	LIBTORRENT_EXCEPTION_SAFE_BEGIN
		
	torrent_status status = info.handle.status();
	
	if (info.resolver_type != no_error_detected)
		return false;

	if (!status.paused && status.state != torrent_status::finished && 
			status.state != torrent_status::seeding && status.state == torrent_status::downloading && 
			(status.num_peers == 0 && status.connect_candidates != 0)) 
	{
		TCORE_TRACE("Torrent error detected : type 'hang downloading' , torrent '%s'", info.handle.name().c_str())
		if (is_problem = (utility::get_current_time() >= (info.add_time + hang_duration_)))
			info.resolver_type = hang_downloading_type;
	}

	LIBTORRENT_EXCEPTION_SAFE_END_(is_problem = false)

	return is_problem;
}

bool connection_error::resolve(resolve_info & info) 
{
	return true;
}

bool connection_error::is_need_resolve(resolve_info & info) 
{
	return false;
}


bool unknown_error::resolve(resolve_info & info) 
{
	return true;
}

bool unknown_error::is_need_resolve(resolve_info & info) 
{
	return false;
}

} } // namspace t2h_core, details

