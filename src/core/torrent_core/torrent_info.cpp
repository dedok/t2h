#include "torrent_info.hpp"

#include <sstream>

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace t2h_core { namespace details {

inline static std::string concat_paths(std::string const & root, std::string const & other) 
{
	boost::filesystem::path full_path(root);
	full_path = full_path / other;
	return full_path.string();
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
		int const last = ex_info->torrent_info->num_files();
		for (int it = 0; it < last; ++it) {
			boost::property_tree::ptree json_child;
			std::string const path = on_path_process(
				concat_paths(
					ex_info->torrent_params.save_path, 
					ex_info->torrent_info->file_at(it).path)
			);

			json_child.put("size", ex_info->torrent_info->file_at(it).size);
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

