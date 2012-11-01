#include "head_reply.hpp"
#include "misc_utility.hpp"
#include "mime_types.hpp"

namespace t2h_core { namespace details {

head_reply::head_reply(
	utility::http_reply::buffer_type & buf_ref, head_reply_param const & param) :
	utility::http_reply(buf_ref), param_(param)
{
}

head_reply::~head_reply() 
{
}
	
bool head_reply::do_formatting_reply() 
{
	using namespace utility;
	
	if (!http_reply::add_status(http_reply::ok))
		return false;

	if (!http_reply::add_header("Accept-Ranges", "bytes"))
		return false;
	
	if (!http_reply::add_header("Content-Type", mime_types::get_file_extension(param_.file_path)))
		return false;

	return http_reply::add_header("Content-Length", 
		safe_lexical_cast<std::string>(param_.expected_file_size));
}

} } // namespace t2h_core, details

