#ifndef HTTP_MIME_TYPES_HPP_INCLUDED
#define HTTP_MIME_TYPES_HPP_INCLUDED

#include <string>

namespace mime_types {

/** Convert a file extension into a MIME type. */
std::string extension_to_type(std::string const & extension);

}

#endif

