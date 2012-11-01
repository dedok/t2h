#include "mime_types.hpp"

namespace mime_types {

struct mapping {
  char const * extension;
  char const * mime_type;
};

static mapping mappings[] =
{
	{ "gif"		, "image/gif" 	},
	{ "htm"		, "text/html" 	},
	{ "html"	, "text/html" 	},
	{ "jpg"		, "image/jpeg" 	},
	{ "png"		, "image/png" 	},
	{ "txt"		, "text/plain"	},
	{ NULL			, NULL 			} 
};

static mapping binary_data = { NULL, "application/octet-stream" };

std::string get_file_extension(boost::filesystem::path const & file_path) 
	{ return file_path.extension().string(); } 

std::string extension_to_type(std::string const & extension) 
{
	/** If type of extension is not in the 'mappings' set 
		then just return the plain text extension, 
		if requested exception is binary data then return empty string. */
	if (extension.empty())
		return binary_data.mime_type;

	for (mapping * m = mappings; 
		m->extension; 
		++m) 
	{
		if (m->extension == extension)
			return m->mime_type;
	} // !for
	return binary_data.mime_type;
}

}

