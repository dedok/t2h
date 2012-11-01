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

static bool file_info_update_avaliable_bytes(file_info & fi) 
{
	int pieces_processed_in_seq = 0;
	for (std::size_t first = fi.last_in_seq; 
		first != fi.pieces_state.size(); 
		++first) 
	{
		if (fi.pieces_state.at(first) != file_info::piece_avaliable)
			break;
		
		boost::int64_t const bytes_av = fi.avaliable_bytes + fi.block_size; 
		(bytes_av < fi.size) ? fi.avaliable_bytes = bytes_av : fi.avaliable_bytes = fi.size;
		fi.last_in_seq = first;
		
		++pieces_processed_in_seq;
	} // for
	return (pieces_processed_in_seq >= fi.max_pieces_processed ? true : false);
}

static inline bool file_info_piece_in_range(file_info const & fi, int piece) 
	{ return (piece >= fi.pieces_range_first && piece <= fi.pieces_range_last); }

/**
 * Public file_info api
 */

file_info file_info_add(file_info::list_type & flist, 
						libtorrent::file_entry const & fe, 
						libtorrent::torrent_info const & ti,
						libtorrent::torrent_handle const & handle,
						int file_index) 
{
	using namespace libtorrent;

	file_info info;
	
	info.pieces_processed = 0;
	info.file_index = file_index;
	int const pieces_range_first = ti.map_file(file_index, 0, 0).piece;
	int const pieces_range_last = ti.map_file(file_index, (std::max)(size_type(fe.size) - 1, size_type(0)), 0).piece;
	int const pieces = (pieces_range_last - pieces_range_first) + 1;
	info.pieces_state.resize(pieces); info.block_size = ti.piece_length();
	info.max_pieces_processed = (pieces < 5) ? pieces : 5;
	info.pieces_range_first = pieces_range_first; info.pieces_range_last = pieces_range_last;
	for (int piece_count = pieces_range_first, count = 0; 
		count < pieces; 
		++count, ++piece_count) 
	{
		info.pieces_state.at(count) = piece_count;
	}
	
	std::string const full_file_path = 
			(boost::filesystem::path(handle.save_path()) / boost::filesystem::path(fe.path)).string(); 
	info.path = full_file_path, info.size = fe.size, info.avaliable_bytes = info.last_in_seq = 0;	
	flist.push_back(info);

	return info;
}

file_info file_info_add_by_index(file_info::list_type & flist, 
								libtorrent::torrent_info const & ti, 
								libtorrent::torrent_handle const & handle,
								int file_index) 
{
	using namespace libtorrent;	
	file_entry const & fe = ti.file_at(file_index);	
	return file_info_add(flist, fe, ti, handle, file_index);	
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

boost::tuple<bool, file_info> file_info_update(file_info::list_type & flist, int piece) 
{
	for (file_info::list_type::iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (file_info_piece_in_range(*first, piece)) {
			std::vector<int>::iterator item = 
				std::find(first->pieces_state.begin(), first->pieces_state.end(), piece);
			if (item != first->pieces_state.end()) {
				++first->pieces_processed;
				*item = file_info::piece_avaliable;
				if (first->pieces_processed >= first->max_pieces_processed) {
					first->pieces_processed = 0;
					bool const seq_is_ready = file_info_update_avaliable_bytes(*first);
					return boost::make_tuple(seq_is_ready, file_info(*first));
				} // if
			}
			break;
		} // if
	} // for
	return boost::make_tuple(false, file_info());
}

file_info file_info_update(file_info::list_type & flist) 
{

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

