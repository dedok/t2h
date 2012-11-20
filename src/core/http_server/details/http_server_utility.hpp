#ifndef HTTP_SERVER_UTILITY_HPP_INCLUDED
#define HTTP_SERVER_UTILITY_HPP_INCLUDED

#include "misc_utility.hpp"
#include "http_utility.hpp"
#include "file_info_buffer.hpp"
#include "http_server_macroses.hpp"

namespace t2h_core { namespace details {

inline static void add_etag_date_headers(std::string & out, details::hc_file_info_ptr const fi) 
{
	std::string const gtm_time = utility::http_get_gmt_time_string(),
		etag = utility::http_etag(fi->file_size, std::time(NULL));
	if (!gtm_time.empty())
		out += "Date: " + gtm_time + "\r\n";
	if (!etag.empty())
		out += "Etag: " + etag + "\r\n";
}

} } // namespace t2h_core, details

#endif 

