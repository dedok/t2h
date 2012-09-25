#include "torrent_info.hpp"
#include "misc_utility.hpp"

#include <sstream>
#include <boost/filesystem/path.hpp>

namespace t2h_core { namespace details {

/**
 * Private-hiddent torrent_ex_info helpers
 */

static inline std::string create_random_path(std::string const & root_path)
{
	boost::filesystem::path save_path = root_path;
	return (save_path / utility::get_random_string()).string();
}

inline static std::string concat_paths(std::string const & root, std::string const & other) 
{
	boost::filesystem::path full_path(root);
	full_path = full_path / other;
	return full_path.string();
}

/**
 * Public torrent_ex_info api
 */

torrent_ex_info::torrent_ex_info() :
	resolver(), 
	last_resolve_checkout(utility::get_current_time()),
	handle(), 
	torrent_params(),
	files_info(),
	save_root(),
	index(0)
{ 
}

bool torrent_ex_info::prepare_f(
	torrent_ex_info::ptr_type ex_info, boost::filesystem::path const & path) 
{
	using namespace libtorrent;

	boost::system::error_code error_code;
	add_torrent_params & torrent_params = ex_info->torrent_params;
	torrent_info_ptr new_torrent_info = new libtorrent::torrent_info(path.string(), error_code);	

	if (!error_code) {
		torrent_params.save_path = create_random_path(ex_info->save_root);
		torrent_params.ti = new_torrent_info;
		torrent_params.flags |= add_torrent_params::flag_paused;
		torrent_params.flags &= ~add_torrent_params::flag_duplicate_is_error;
		torrent_params.flags |= add_torrent_params::flag_auto_managed;
		torrent_params.storage_mode = (storage_mode_t)storage_mode_sparse;
		return true;
	}
	
	return false;
}

void torrent_ex_info::prepare_u(torrent_ex_info::ptr_type ex_info, std::string const & url) 
{
	using namespace libtorrent;
	
	add_torrent_params & torrent_params = ex_info->torrent_params;
	torrent_params.save_path = create_random_path(ex_info->save_root);
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
	ex_info->save_root = save_root.string();
	if (!torrent_ex_info::prepare_f(ex_info, path)) 
		return false;
	if (!torrent_ex_info::prepare_sandbox(ex_info)) 
		return false;
	return true;
}

std::string torrent_info_to_json(
	torrent_ex_info_ptr const ex_info, 
	boost::function<std::string(std::string const &)> on_path_process) 
{
	std::stringstream json_text_stream;
	boost::property_tree::ptree json_root;
	boost::property_tree::ptree out_json;
	try 
	{
		libtorrent::torrent_info const & info = ex_info->handle.get_torrent_info();
		int const last = info.num_files();
		for (int it = 0; it < last; ++it) {
			boost::property_tree::ptree json_child;
			std::string const path = on_path_process(
				concat_paths(ex_info->torrent_params.save_path, info.file_at(it).path));

			json_child.put("size", info.file_at(it).size);
			json_child.put("path", path);
			json_child.put("id", it);
			json_root.push_back(std::make_pair("", json_child));
		}	
		
		out_json.put_child(
			boost::property_tree::ptree::path_type("torrent_info"),
			json_root);

		boost::property_tree::write_json(json_text_stream, out_json);
	} 
	catch (std::exception const & expt) { /**/ }
	
	return json_text_stream.str();
}

} } // namesapce t2h_core, details

