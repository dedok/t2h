#include "torrent_info.hpp"
#include "misc_utility.hpp"
#include "torrent_core_utility.hpp"

#include <limits>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <boost/filesystem/path.hpp>

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101) 
#endif

//#define T2H_DEEP_DEBUG

namespace t2h_core { namespace details {

/**
 * Private hidden torrent_ex_info api and file_info api
 */

static void replace_slashes(std::string & path) 
{	
	/* replace slashes in path for json output */
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

static void normalize_slashes(std::string & path) 
{
	/* WORKARAUND for windows platform, should replace this solution ASAP*/
#if defined(WIN32)
	for (std::size_t it = 0, last = path.size(); it < last; ++it) {
		if (path.at(it) == '\\')
			path.at(it) = '/';
	} 
#endif // WIN32
}

static inline void file_info_update_counters_(file_info_ptr fi) 
{
	if (fi->total_pieces_download_count + 1 > fi->pieces) { 
		fi->total_pieces_download_count = fi->pieces;
		return;
	}
	++fi->total_pieces_download_count; 
	++fi->pieces_download_count;
	++fi->last_av_pieces_pos; 				
		
	if (fi->recheck_av != file_info::off_recheck)
		++fi->recheck_av;
}

static inline bool file_info_piece_in_range(file_info_ptr fi, int piece) 
	{ return (piece >= fi->pieces_range_first && piece <= fi->pieces_range_last); }

static void file_info_clear_priority(file_info_ptr const fi, libtorrent::torrent_handle & handle) 
{
	for (std::size_t it = fi->pieces_range_first, last = fi->pieces_range_last; it > last; ++it)
		handle.piece_priority(it, file_info::off_prior);
}

static inline int file_info_get_download_offset(file_info_ptr const fi, int max_partial_download_size) 
{			
		int const dow_offset = max_partial_download_size / fi->block_size;
		if (dow_offset == 1) return 10; 
		return (max_partial_download_size >= fi->size || dow_offset >= fi->pieces) ?
				fi->pieces : dow_offset;
}


/**
 * Public file_info api
 */

static inline void file_info_trace_dump(file_info_ptr const fi) 
{
#if defined(T2H_DEEP_DEBUG)
	TCORE_TRACE("\ntotal pieces '%i'\ntotal downloaded '%i'\npieces download count '%i'\n",
		fi->pieces, fi->total_pieces_download_count, fi->pieces_download_count)
	
	TCORE_TRACE("'\npieces range first '%i'\npieces range last '%i'\n",
		fi->pieces_range_first, fi->pieces_range_last)

	TCORE_TRACE("'\navaliable bytes '%i'\npath '%s'\nfile index '%i'\n",
		fi->avaliable_bytes, fi->path.c_str(), fi->file_index)	
	
	TCORE_TRACE("\nblock size '%i'\nchocked range '%i'\n", 
			fi->block_size, fi->chocked_range)
#endif // T2H_DEEP_DEBUG
}

file_info_ptr file_info_add(file_info::list_type & flist, 
						libtorrent::file_entry const & fe, 
						libtorrent::torrent_info const & ti,
						libtorrent::torrent_handle const & handle,
						int file_index,
						int max_partial_download_size) 
{
	using namespace libtorrent;

	file_info_ptr info(new file_info());	
	
	int const pieces_range_first = ti.map_file(file_index, 0, 0).piece;
	int const pieces_range_last = ti.map_file(file_index, (std::max)(size_type(fe.size) - 1, size_type(0)), 0).piece;
	int const block_size = handle.get_torrent_info().piece_length();
	
	/* initialize file information */
	info->file_index = file_index;
	info->path = handle.save_path() + "/" + fe.path; 
	normalize_slashes(info->path);
	info->size = fe.size; 
	info->block_size = (block_size > info->size) ? info->size : block_size;

	/* initialize file pieces information */
	info->pieces = 1 * (pieces_range_last - pieces_range_first);
	info->pieces_range_first = pieces_range_first; 
	info->pieces_range_last = pieces_range_last;
	info->end_av_pos = info->chocked_range = file_info_get_download_offset(info, max_partial_download_size);
	info->recheck_av = file_info::off_recheck;
	info->last_av_pos = info->last_av_pieces_pos = info->pieces_download_count = info->avaliable_bytes = info->total_pieces_download_count = 0;
	// Just for sure make vector of avaliable pieces more than pieces for this file  
	info->av_pieces.resize(info->pieces + 1);
	// I believe more than INT_MAX pieces in torrent is seems unreal
	std::fill(info->av_pieces.begin(), info->av_pieces.end(), std::numeric_limits<int>::max());

	flist.push_back(info);

#if defined(T2H_DEEP_DEBUG)
	file_info_trace_dump(info);
#endif // T2H_DEEP_DEBUG

	return info;
} 

file_info_ptr file_info_add_by_index(file_info::list_type & flist, 
								libtorrent::torrent_info const & ti, 
								libtorrent::torrent_handle const & handle,
								int file_index,
								int max_partial_download_size) 
{
	using namespace libtorrent;	
	file_entry const & fe = ti.file_at(file_index);	
	return file_info_add(flist, fe, ti, handle, file_index, max_partial_download_size);	
}

file_info_ptr file_info_bin_search(file_info::list_type const & flist, int piece) 
{
	for (file_info::list_type::const_iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if (file_info_piece_in_range(*first, piece))  
			return *first;
	} // for
	return file_info_ptr();
}

file_info_ptr file_info_search(file_info::list_type const & flist, std::string const & path) 
{
	std::string path_ = path;
	normalize_slashes(path_);
	for (file_info::list_type::const_iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if ((*first)->path == path_) 
			return *first;
	} // for
	return file_info_ptr();
}

file_info_ptr file_info_search_by_index(file_info::list_type const & flist, int index) 
{
	// TODO improve search by index to 0(n) time
	for (file_info::list_type::const_iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if ((*first)->file_index == index) 
			return *first;
	} // for
	return file_info_ptr();
}

void file_info_set_pieces_priority(
	file_info::list_type & flist, libtorrent::torrent_handle & handle, int index, int priority) 
{
	/* TODO reimplement this */
#if 0
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
#endif 
}

file_info_ptr file_info_update(file_info::list_type & flist, libtorrent::torrent_handle & handle, int piece) 
{
	/*  The bittorrent not sequential. But we can cheat a bit to make the bittorrent protocol more sequential :
	 	first what need is choking + a lot of counters + download set(sorted) to always known how many 
		sequential pieces we downloaded. 
		TODO Follow algo. work complex, need performans improvements */
	for (file_info::list_type::iterator first_ = flist.begin(), last = flist.end();
		first_ != last; 
		++first_)
	{
		file_info_ptr first = *first_; 
		if (file_info_piece_in_range(first, piece)) {
#if defined(T2H_DEEP_DEBUG)
			TCORE_TRACE("pieces downloaded '%i', for file '%s'", piece, first->path.c_str())
#endif // T2H_DEEP_DEBUG
			first->av_pieces.at(first->last_av_pieces_pos) = piece;
			file_info_update_counters_(first);

			if ((first->pieces_download_count >= first->chocked_range || first->recheck_av > file_info::recheck_limit)
				&& first->total_pieces_download_count < first->pieces) 
			{
				std::partial_sort(first->av_pieces.begin(), 
					first->av_pieces.begin() + first->total_pieces_download_count, first->av_pieces.end());
				
				for (std::size_t it = first->last_av_pos; it < first->end_av_pos; ++it) {
					if (first->av_pieces.size() == it)
						break;

					if (first->av_pieces.at(it) == (first->av_pieces.at(it + 1) - 1)) 
						first->last_av_pos = it;
					else if (first->recheck_av == file_info::off_recheck) {
#if defined(T2H_DEEP_DEBUG)
						TCORE_TRACE("set maximum prior for pieces from '%i to '%i'", first->last_av_pos, first->end_av_pos)
#endif // T2H_DEEP_DEBUG
						for (std::size_t piece_inx = first->last_av_pos; piece_inx != first->end_av_pos; ++piece_inx)
							handle.piece_priority(piece_inx, file_info::max_prior);
						break;
					}
				} // for

				if (first->last_av_pos >= first->end_av_pos - 1) {
#if defined(T2H_DEEP_DEBUG)
					TCORE_TRACE("range complete '%i' <> '%i'", first->last_av_pos, first->end_av_pos)
#endif // T2H_DEEP_DEBUG
					first->last_av_pos = first->end_av_pos;
					first->end_av_pos = first->last_av_pos + first->chocked_range; 
					first->pieces_download_count = 0;
					first->recheck_av = file_info::off_recheck;
				} else {
#if defined(T2H_DEEP_DEBUG)
					TCORE_TRACE("force recheck for range '%i' <> '%i'", first->last_av_pos, first->end_av_pos)
#endif // T2H_DEEP_DEBUG
					++first->recheck_av;
				}
				boost::int64_t const curr_avb = first->last_av_pos * first->block_size;
				if (first->avaliable_bytes < curr_avb) {
					first->avaliable_bytes = curr_avb;
					return first;
				}
			} // if
#if defined(T2H_DEEP_DEBUG) 
				file_info_trace_dump(first);
#endif // T2H_DEEP_DEBUG
			break;
		} // if in big range
	} // for
	return file_info_ptr();
}

void file_info_reinit(file_info_ptr fi) 
{
	/* re-initialize file pieces information */
	fi->recheck_av = file_info::off_recheck;
	fi->last_av_pos = fi->last_av_pieces_pos = fi->pieces_download_count = fi->total_pieces_download_count = 0;
	std::fill(fi->av_pieces.begin(), fi->av_pieces.end(), std::numeric_limits<int>::max());

}

void file_info_remove(file_info::list_type & flist, std::string const & path) 
{
	for (file_info::list_type::iterator first = flist.begin(), last = flist.end();
		first != last; 
		++first)
	{
		if ((*first)->path == path) {	
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
