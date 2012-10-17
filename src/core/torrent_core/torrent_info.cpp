#include "torrent_info.hpp"
#include "misc_utility.hpp"
#include "torrent_core_utility.hpp"

#include <sstream>
#include <boost/filesystem/path.hpp>

#if defined(WIN32)
#	pragma warning(push)
#	pragma warning(disable : 4101) 
#endif

namespace t2h_core { namespace details {

/**
 * Private hidden torrent_ex_info api
 */

static inline void replace_slashes(std::string & path) 
{
	std::string const slashes = "/";
	for(std::string::size_type first = 0; 
		first < path.size(); 
		++first)
	{
		if (path.at(first) == '/') {
			std::cout << first << std::endl;
			path.insert(first + 1, slashes);
			first+=2;
		} // if
	}
}

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
	std::cout << save_root.string() << std::endl << path.string() << std::endl;
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

