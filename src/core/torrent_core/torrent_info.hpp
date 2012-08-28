#ifndef TORRENTS_INFO_EX_HPP_INCLUDED
#define TORRENTS_INFO_EX_HPP_INCLUDED

#include <map>
#include <vector>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#if defined (__GNUG__)
#pragma GCC system_header
#endif

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace t2h_core { namespace details {

static int const invalid_torrent_id = -1;

struct file_ex_info {
	static std::size_t const off_prior = 0;
	static std::size_t const normal_prior = 1;
	static std::size_t const max_prior = 5;
};

typedef std::vector<file_ex_info> files_info_list_type;
typedef files_info_list_type::iterator feil_iterator;
typedef files_info_list_type::const_iterator feil_const_iterator;

struct torrent_ex_info {	
	typedef boost::scoped_ptr<libtorrent::torrent_info> torrent_info_ptr;

	torrent_ex_info() :
		files_info(),
		save_path(),
		torrent_info(),
		handle(), 
		torrent_params() 
	{ }
 	
	files_info_list_type files_info;
	std::string save_path;
	torrent_info_ptr torrent_info;
	libtorrent::torrent_handle handle;
	libtorrent::add_torrent_params torrent_params;
};

typedef boost::shared_ptr<torrent_ex_info> torrent_ex_info_ptr;
typedef std::map<int, torrent_ex_info_ptr> torrents_map_type; 

struct default_on_path_process {
	inline std::string operator()(std::string const & p) { return p; } 
};

std::string torrent_info_to_json(
	torrent_ex_info_ptr ex_info, 
	boost::function<std::string(std::string const &)> on_path_process = default_on_path_process());

} } // namespace t2h_core, details

#endif

