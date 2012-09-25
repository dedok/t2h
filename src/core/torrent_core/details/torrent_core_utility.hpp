#ifndef TORRENT_CORE_UTILITY_HPP_INCLUDED
#define TORRENT_CORE_UTILITY_HPP_INCLUDED

#include <string>
#include <vector>

namespace t2h_core { namespace details {

int save_file(std::string const & filename, std::vector<char> & bytes);

std::string concat_paths(std::string const & root, std::string const & other); 

std::string create_random_path(std::string const & root_path, std::string & random_string);

} } // namespace t2h_core, namespace details

#endif

