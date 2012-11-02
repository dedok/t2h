#include "torrent_info.hpp"
#include "misc_utility.hpp"
#include "torrent_core_utility.hpp"

#include <sstream>
#include <algorithm>
#include <boost/filesystem/path.hpp>

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101) 
#endif

namespace t2h_core { namespace details {

/**
 * Private hidden torrent_ex_info api and file_info api
 */
static void replace_slashes(std::string & path) 
{
	std::string const slashes = "\\";
	for(std::string::size_type first = 0; 
		first < path.size(); 
		++first)
	{
		if (path.at(first) == '/') {
			path.at(first) = '\\';
			path.insert(first + 1, slashes);
			first+=2;
		} else if (path.at(first) == '\\') {
			path.insert(first + 1, slashes);
			first+=2;
		}
	}
}

static inline bool file_info_piece_in_range(file_info const & fi, int piece) 
	{ return (piece >= fi.pieces_range_first && piece <= fi.pieces_range_last); }

static void file_info_set_pieces_priority_(file_info & info, libtorrent::torrent_handle & handle) 
{
	int const req_range_last = info.pieces_download_offset_min + info.pieces_download_offset;
	int const last = (req_range_last > info.pieces_range_last) ? info.pieces_range_last : req_range_last;
	for (int it = info.pieces_download_offset_min; it > last; ++it)
		handle.piece_priority(it, file_info::max_prior);
	info.pieces_download_offset_min = last;
}

static void file_info_clear_priority(file_info & finfo, libtorrent::torrent_handle & handle) 
{
	for (int it = finfo.pieces_range_first, last = finfo.pieces_range_last; it > last; ++it)
		handle.piece_priority(it, file_info::off_prior);
}

static inline int file_info_get_download_offset(file_info const & info, int max_partial_download_size) 
{		
	int size = info.block_size;
	if (max_partial_download_size > info.size)
		return info.pieces;
	
	int const dow_offset = max_partial_download_size / info.block_size;
	if (dow_offset >= info.pieces)
		return info.pieces;
	
	return dow_offset;
}


/**
 * Public file_info api
 */

file_info file_info_add(file_info::list_type & flist, 
						libtorrent::file_entry const & fe, 
						libtorrent::torrent_info const & ti,
						libtorrent::torrent_handle const & handle,
						int file_index,
						int max_partial_download_size) 
{
	using namespace libtorrent;

	file_info info;	

	int const pieces_range_first = ti.map_file(file_index, 0, 0).piece;
	int const pieces_range_last = ti.map_file(file_index, (std::max)(size_type(fe.size) - 1, size_type(0)), 0).piece;
	int const block_size = handle.status().block_size;
	
	/* initialize file information */
	info.file_index = file_index;
	info.path = (boost::filesystem::path(handle.save_path()) / boost::filesystem::path(fe.path)).string(); 
	info.size = fe.size; 
	info.block_size = (block_size > info.size) ? info.size : block_size;

	/* initialize file pieces information */
	info.pieces = (pieces_range_last - pieces_range_first) + 1;
	info.pieces_download_offset_min = info.pieces_range_first = pieces_range_first; 
	info.pieces_range_last = pieces_range_last;
	info.pieces_download_offset = file_info_get_download_offset(info, max_partial_download_size); 
	info.pieces_download_count = info.avaliable_bytes = info.total_pieces_download_count = 0;
	flist.push_back(info);
	return info;
} 

file_info file_info_add_by_index(file_info::list_type & flist, 
								libtorrent::torrent_info const & ti, 
								libtorrent::torrent_handle const & handle,
								int file_index,
								int max_partial_download_size) 
{
	using namespace libtorrent;	
	file_entry const & fe = ti.file_at(file_index);	
	return file_info_add(flist, fe, ti, handle, file_index, max_partial_download_size);	
}

bool file_info_bin_search(file_info::list_type const & flist, int piece, file_info & info) 
{
	for (file_info::list_type::const_iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (file_info_piece_in_range(*first, piece)) { 
			info = *first;
			return true;
		}
	} // for
	return false;
}

bool file_info_search(file_info::list_type const & flist, std::string const & path, file_info & info) 
{
	for (file_info::list_type::const_iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (first->path == path) {
			info = *first;
			return true;
		}
	} // for
	return false;
}

bool file_info_search_by_index(file_info::list_type const & flist, int index, file_info & info) 
{
	for (file_info::list_type::const_iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (first->file_index == index) {
			info = *first;
			return true;
		}
	} // for
	return false;
}

void file_info_set_pieces_priority(
	file_info::list_type & flist, libtorrent::torrent_handle & handle, int index, bool clear_priority_first) 
{
	for (file_info::list_type::iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (first->file_index == index) {
			if (clear_priority_first)
				file_info_clear_priority(*first, handle);
			file_info_set_pieces_priority_(*first, handle);
			return;
		}
	} // for
}

boost::tuple<bool, file_info> 
	file_info_update(file_info::list_type & flist, libtorrent::torrent_handle & handle, int piece) 
{
	for (file_info::list_type::iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (file_info_piece_in_range(*first, piece)) {	
			++first->pieces_download_count;
			if (first->total_pieces_download_count < first->pieces) 
				++first->total_pieces_download_count;

			if (first->pieces_download_count > first->pieces_download_offset || 
				first->total_pieces_download_count == first->pieces) 
			{	
				first->avaliable_bytes = first->total_pieces_download_count * first->block_size;
				file_info_set_pieces_priority_(*first, handle);
				first->pieces_download_count = 0;
				return boost::make_tuple(true, file_info(*first));
			} // if
		} // if
	} // for
	return boost::make_tuple(false, file_info());
}

void file_info_remove(file_info::list_type & flist, std::string const & path) 
{
	for (file_info::list_type::iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (first->path == path) {
			flist.erase(first);
			return;
		}
	} // for
}

void file_info_remove(file_info::list_type & flist, int piece) 
{
	for (file_info::list_type::iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (file_info_piece_in_range(*first, piece)) {
			flist.erase(first);
			return;
		}
	} // for
}

void file_info_reset(file_info::list_type & flist) 
	{ flist.clear(); }

/**
 * Public torrent_ex_info api
 */

torrent_ex_info::torrent_ex_info() :
	resolver(), 
	last_resolve_checkout(utility::get_current_time()),
	handle(), 
	torrent_params(),
	sandbox_dir_name(),
	index(0)
{ 
}

bool torrent_ex_info::prepare_f(
	torrent_ex_info::ptr_type ex_info, 
	boost::filesystem::path const & save_root,
	boost::filesystem::path const & path) 
{
	using namespace libtorrent;

	boost::system::error_code error_code;
	add_torrent_params & torrent_params = ex_info->torrent_params;
	torrent_info_ptr new_torrent_info = new libtorrent::torrent_info(path.string(), error_code);	

	if (!error_code) {
		torrent_params.save_path = create_random_path(save_root.string(), ex_info->sandbox_dir_name);
		torrent_params.ti = new_torrent_info;
		torrent_params.flags |= add_torrent_params::flag_paused;
		torrent_params.flags &= ~add_torrent_params::flag_duplicate_is_error;
		torrent_params.flags |= add_torrent_params::flag_auto_managed;
		torrent_params.storage_mode = (storage_mode_t)storage_mode_sparse;
		return true;
	}
	
	return false;
}

void torrent_ex_info::prepare_u(
	torrent_ex_info::ptr_type ex_info, 
	boost::filesystem::path const & save_root,
	std::string const & url) 
{
	using namespace libtorrent;
	
	add_torrent_params & torrent_params = ex_info->torrent_params;
	torrent_params.save_path = create_random_path(save_root.string(), ex_info->sandbox_dir_name);
	torrent_params.flags |= add_torrent_params::flag_paused;
	torrent_params.flags &= ~add_torrent_params::flag_duplicate_is_error;
	torrent_params.flags |= add_torrent_params::flag_auto_managed;
	torrent_params.url = url;
}

bool torrent_ex_info::prepare_sandbox(torrent_ex_info::ptr_type ex_info) 
{
	try 
	{
		using namespace libtorrent;
		
		add_torrent_params & torrent_params = ex_info->torrent_params;	
		if (!boost::filesystem::exists(torrent_params.save_path) && 
			!torrent_params.save_path.empty()) 
		{ 
			return boost::filesystem::create_directory(torrent_params.save_path);
		}
	} catch (boost::filesystem3::filesystem_error const & expt) { /* do nothing */ }
	return false;
}

bool torrent_ex_info::initialize_f(
	torrent_ex_info::ptr_type ex_info, 
	boost::filesystem::path const & save_root,
	boost::filesystem::path const & path) 
{
	if (!torrent_ex_info::prepare_f(ex_info, save_root, path)) 
		return false;
	if (!torrent_ex_info::prepare_sandbox(ex_info)) 
		return false;
	return true;
}

std::string torrent_info_to_json(
	torrent_ex_info_ptr const ex_info, 
	boost::function<std::string(std::string const &)> on_path_process) 
{
	std::string result;
	try 
	{
		libtorrent::torrent_info const & info = ex_info->handle.get_torrent_info();
		int const last = info.num_files();
		for (int it = 0; it < last; ++it) {
			std::string path = on_path_process(
				libtorrent::combine_path(ex_info->torrent_params.save_path, info.file_at(it).path));
			replace_slashes(path);
			result += "{\n\"size\": " + utility::safe_lexical_cast<std::string>(info.file_at(it).size) + ",\n";
			result += "\"path\": \"" +  path + "\",\n";
			result += "\"id\": " + utility::safe_lexical_cast<std::string>(it); 
			(it != last - 1) ? result += "\n},\n" : result += "\n}\n";
		} // for
	}	
	catch (std::exception const & expt) { return std::string(); }
	return std::string("[\n" + result + "]");
}

} } // namesapce t2h_core, details

#if defined(WIN32)
#	pragma warning(pop)
#endif

