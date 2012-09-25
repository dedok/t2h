#ifndef TORRENTS_INFO_EX_HPP_INCLUDED
#define TORRENTS_INFO_EX_HPP_INCLUDED

#include "base_resolver.hpp"
#include "torrent_core_future.hpp"

#include <map>
#include <vector>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

#if defined (__GNUG__)
#	pragma GCC system_header
#endif

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace t2h_core { namespace details {

/**
 *	File extended information
 */
struct file_ex_info {
	static std::size_t const off_prior = 0;
	static std::size_t const normal_prior = 1;
	static std::size_t const max_prior = 5;
};

/**
 * Torrent extended informantion & functionality
 */
struct torrent_ex_info {	
	typedef boost::shared_ptr<torrent_ex_info> ptr_type;
	typedef boost::intrusive_ptr<libtorrent::torrent_info> torrent_info_ptr;

	torrent_ex_info();
	
	/* Extended functionality */
	static bool initialize_f(
		ptr_type ex_info, boost::filesystem::path const & save_root, boost::filesystem::path const & path);
	static bool prepare_f(
		ptr_type ex_info, boost::filesystem::path const & save_root, boost::filesystem::path const & path);
	static void prepare_u(
		ptr_type ex_info, boost::filesystem::path const & save_root, std::string const & url);
	static bool prepare_sandbox(ptr_type ex_info);
	
	/* Extended data filds */
	base_resolver_ptr resolver;									// Error reslover
	boost::posix_time::time_duration last_resolve_checkout;		// Error checking duration 
	details::torrent_core_future_ptr future;					// Future callback

	libtorrent::torrent_handle handle;							// libtorrent torrent handle
	libtorrent::add_torrent_params torrent_params;				// libtorrent add torrent params

	std::string sandbox_dir_name;								// sandbox directory name
	std::size_t index;											// Torrent index(eg hash)
};

typedef torrent_ex_info::ptr_type torrent_ex_info_ptr;

/**
 * Torrent extended info helpers 
 */
struct default_on_path_process {
	inline std::string operator()(std::string const & p) { return p; } 
};

std::string torrent_info_to_json(
	torrent_ex_info_ptr ex_info, 
	boost::function<std::string(std::string const &)> on_path_process = default_on_path_process());

} } // namespace t2h_core, details

#endif

