#ifndef HTTP_MIME_TYPES_HPP_INCLUDED
#define HTTP_MIME_TYPES_HPP_INCLUDED

#include <string>
#include <boost/filesystem.hpp>

namespace mime_types {

/** Get file extension */
std::string get_file_extension(boost::filesystem::path const & file_path);

/** Convert a file extension into a MIME type. */
std::string extension_to_type(std::string const & extension);

}

#endif

