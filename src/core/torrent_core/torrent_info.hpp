#ifndef TORRENTS_INFO_EX_HPP_INCLUDED
#define TORRENTS_INFO_EX_HPP_INCLUDED

#include <map>
#include <vector>

#include <libtorrent/config.hpp>
#include <libtorrent/session.hpp>

namespace t2h_core { namespace details {

static int const invalid_torrent_id = -1;

struct file_ex_info {
	static const std::size_t off_prior = 0;
	static const std::size_t min_prior = 1;
	static const std::size_t max_prior = 5;
};

typedef std::vector<file_ex_info> files_info_list_type;
typedef files_info_list_type::iterator feil_iterator;
typedef files_info_list_type::const_iterator feil_const_iterator;

struct torrent_ex_info {	
	torrent_ex_info() : 
		files_info(),
		handle(), 
		torrent_params() 
	{ }
 	
	files_info_list_type files_info;	
	libtorrent::torrent_handle handle;
	libtorrent::add_torrent_params torrent_params;
};

typedef boost::shared_ptr<torrent_ex_info> torrent_ex_info_ptr;
typedef std::map<int, torrent_ex_info_ptr> torrents_map_type; 

} } // namespace t2h_core, details

#endif

