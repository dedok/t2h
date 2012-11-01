#include "root_reply.hpp"
#include "misc_utility.hpp"

namespace t2h_core { namespace details {

root_reply::root_reply(utility::http_reply::buffer_type & buf_ref, root_reply_param const & param) 
	: utility::http_reply(buf_ref), param_(param) 
{ 
}

root_reply::~root_reply() 
{ 
}
		
bool root_reply::do_formatting_reply() 
{
	using namespace utility;
	
	std::string const root_request_content = "<html><body>t2h http server status : OK</body></html>";
	std::string::size_type const rrc_size = root_request_content.size(); 

	// TODO add better answer on root request
	if (!add_status(http_reply::ok))
		return false;
	
	if (!add_header("Content-Length", utility::safe_lexical_cast<std::string>(rrc_size)))
		return false;

	if (!add_header("Content-Type", "text/html"))
		return false;

	return add_content(root_request_content.c_str(), rrc_size);
}

} } // namespace t2h_core, details

