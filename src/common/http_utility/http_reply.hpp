#ifndef HTTP_SERVER_REPLY_HPP_INCLUDED
#define HTTP_SERVER_REPLY_HPP_INCLUDED

#include "http_header.hpp"

#include <boost/cstdint.hpp>
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
	
	/**
	 *
	 */
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
	
	http_reply(buffer_type & buffer);
	virtual ~http_reply();
	
	/**
	 *
	 */
	virtual bool do_formatting_reply() 
		{ return false; }
	
	virtual bool stock_reply(status_type status);
	/**
	 *
	 */		
	bool add_status(status_type status);
	bool add_header(std::string const & name, std::string const & value);	
	bool add_header(http_header const & header);
	bool add_content(char const * content, std::size_t content_size);
	void add_crlf();

	/**
	 *
	 */
	inline buffer_type & get_buffer() { return buf_ref_; }
	inline buffer_type const & get_buffer() const { return buf_ref_; }

	void reset_buffer();
	void enable_buf_realocation(bool state = true);

protected :
	bool mutable enable_buf_realocation_;

private :
	void add_header_directly(std::string const & name, std::string const & value);
	void add_header_directly(http_header const & header);

	buffer_type & buf_ref_;

};

} // namespace utlility

#endif 

