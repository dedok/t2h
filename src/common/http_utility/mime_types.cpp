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
	{ 0			, 0 			} 
};

static mapping plain_text = { 0, "text/plain" };
static mapping binary_data = { 0, "" };

std::string extension_to_type(std::string const & extension) 
{
	/** If type of extension is not in the 'mappings' set 
		then just return the plain text extension, 
		if requested exception is binary data then return empty string. */
	for (mapping * m = mappings; 
		m->extension; 
		++m) 
	{
		if (m->extension == extension)
			return m->mime_type;
	} // !for
	return plain_text.mime_type;
}

}

