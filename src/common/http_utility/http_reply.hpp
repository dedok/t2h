#ifndef HTTP_SERVER_REPLY_HPP_INCLUDED
#define HTTP_SERVER_REPLY_HPP_INCLUDED

#include "http_header.hpp"

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

namespace utility {

/**
 *	Server fingerprint have follow format :
 *	REPLY HTTP TYPE\r\n
 *	Server: fingerprint::name/fingerprint::version (OS type)\r\n
 *	REPLY HTTP HEADERS\r\n
 *	REPLY CONTENT
 */
struct fingerprint {
	std::string name;			// server name
	std::string version;		// string version represintation
	std::size_t hex_version;	// hex_version of the string version
	bool enable_fingerprint;	// enable/disable finger print at the end of http reply message(true default)
};

/**
 * htpp reply
 */
class http_reply : private boost::noncopyable {
public :
	typedef std::vector<char> buffer_type;
	typedef buffer_type::iterator buffer_iter_type;
	typedef buffer_type::const_iterator const_buffer_iter_type;

	enum status_type {
		ok = 200,
		created = 201,
		accepted = 202,
		no_content = 204,
		partial_content = 206,
		multiple_choices = 300,
		moved_permanently = 301,
		moved_temporarily = 302,
		not_modified = 304,
		bad_request = 400,
		unauthorized = 401,
		forbidden = 403,	
		not_found = 404,
		internal_server_error = 500,
		not_implemented = 501,
		bad_gateway = 502,
		service_unavailable = 503
	};
	
	enum formating_result {
		formating_succ = 0x1,	
		file_not_exist,
		io_error,
		buffer_error,
		unknown_error,
	};

	http_reply(fingerprint & fp_ref, buffer_type & buffer);
	~http_reply();

	void add_header(std::string const & name, std::string const & value);	
	void add_header(http_header const & header);
	
	bool add_content_from_file(boost::filesystem::path const & file_path, boost::int64_t from, boost::int64_t to);
	bool add_content_directly(char const * content, std::size_t content_size);	
	
	void set_status(status_type status);
	status_type get_status() const;

	formating_result format_partial_content(boost::filesystem::path const & req_path, boost::int64_t start, boost::int64_t end);
	void stock_reply(status_type status);
	void reset_buffer();
	
	void set_no_auto_generate_headers(bool state) const;
	void enable_fingerprint(bool state = true);

private :
	void add_crlf_directly();
	void add_header_directly(std::string const & name, std::string const & value);
	void add_header_directly(http_header const & header);
	void add_headers();

	formating_result fill_content_from_file();

	struct {
		boost::filesystem::path file_path;
		boost::int64_t file_size;
		boost::int64_t from;
		boost::int64_t to;
		boost::int64_t size_for_reading;
	} file_info_;

	status_type status_;
	header_list_type headers_;
	buffer_type & buf_ref_;
	fingerprint mutable & fp_ref_;
	bool mutable no_auto_generate_headers_;
};

} // namespace utlility

#endif 

