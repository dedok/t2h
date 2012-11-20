#include "http_server_ostream_policy.hpp"

namespace t2h_core { namespace details {

/**
 * Public http_server_ostream_policy api
 */

http_server_ostream_policy::http_server_ostream_policy(http_server_ostream_policy_params const & params) 
	: ostream_impl_(), base_params_(params) 
{ 

}	

http_server_ostream_policy::~http_server_ostream_policy() 
{

}

bool http_server_ostream_policy::perform(http_core_reply & reply, http_data & hd) 
{
	std::string reply_headers;
	bool is_need_perform_content = false, state = false;

	if (ostream_impl_) {
		boost::tie(reply_headers, is_need_perform_content) = reply.get_reply_headers(hd);	
		state = (ostream_impl_->write(reply_headers.c_str(), reply_headers.size()) > 0);
		
		if (is_need_perform_content && state)  
			state = write_content_impl(hd);
	} // if
	
	if (base_params_.reset_stream_at_end_of_io)
		ostream_impl_.reset();

	return state;
}

} } // namespace t2h_core, details

