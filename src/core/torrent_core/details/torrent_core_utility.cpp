#include "torrent_core_utility.hpp"

#include "misc_utility.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/bitfield.hpp>

#include <boost/filesystem.hpp>

namespace t2h_core { namespace details {

int save_file(std::string const & filename, std::vector<char> & bytes)
{
	libtorrent::file file;
	boost::system::error_code error_code;
	if (!file.open(filename, libtorrent::file::write_only, error_code)) { 
		if (error_code) 
			return -1;
		libtorrent::file::iovec_t filb = { &bytes[0], bytes.size() };
		libtorrent::size_type const written = file.writev(0, &filb, 1, error_code);
		return (error_code ? -3 : static_cast<int>(written));
	}
	return -1;
}

std::string concat_paths(std::string const & root, std::string const & other) 
{
	boost::filesystem::path full_path(root);
	full_path = full_path / other;
	return full_path.string();
}

std::string create_random_path(std::string const & root_path, std::string & random_string)
{
	random_string = utility::get_random_string();
	boost::filesystem::path path = root_path;
	return (path / random_string).string();
}

} } // namespace t2h_core, details

