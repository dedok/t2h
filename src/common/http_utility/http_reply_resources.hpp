#ifndef HTTP_REPLY_RESOURCES_HPP_INCLUDED
#define HTTP_REPLY_RESOURCES_HPP_INCLUDED

#include "http_reply.hpp"

#include <string>
#include <utility>

#define MAKE_CONST_STRING(x, value) std::string const x = value;

namespace utility {

namespace status_strings {

MAKE_CONST_STRING(ok, "HTTP/1.1 200 OK\r\n")
MAKE_CONST_STRING(created, "HTTP/1.1 201 Created\r\n")
MAKE_CONST_STRING(accepted, "HTTP/1.1 202 Accepted\r\n")
MAKE_CONST_STRING(no_content, "HTTP/1.1 204 No Content\r\n")
MAKE_CONST_STRING(partial_content, "HTTP/1.1 206 Partial Content\r\n")
MAKE_CONST_STRING(multiple_choices, "HTTP/1.1 300 Multiple Choices\r\n")
MAKE_CONST_STRING(moved_permanently, "HTTP/1.1 301 Moved Permanently\r\n")
MAKE_CONST_STRING(moved_temporarily, "HTTP/1.1 302 Moved Temporarily\r\n")
MAKE_CONST_STRING(not_modified, "HTTP/1.1 304 Not Modified\r\n")
MAKE_CONST_STRING(bad_request, "HTTP/1.1 400 Bad Request\r\n")
MAKE_CONST_STRING(unauthorized, "HTTP/1.1 401 Unauthorized\r\n")
MAKE_CONST_STRING(forbidden, "HTTP/1.1 403 Forbidden\r\n")
MAKE_CONST_STRING(not_found, "HTTP/1.1 404 Not Found\r\n")
MAKE_CONST_STRING(internal_server_error, "HTTP/1.1 500 Internal Server Error\r\n")
MAKE_CONST_STRING(not_implemented, "HTTP/1.1 501 Not Implemented\r\n")
MAKE_CONST_STRING(bad_gateway, "HTTP/1.1 502 Bad Gateway\r\n")
MAKE_CONST_STRING(service_unavailable, "HTTP/1.1 503 Service Unavailable\r\n")

static inline void cast_helper(http_reply::buffer_type & buffer, std::string const & message) 
{
	http_reply::buffer_type::size_type const b_size = buffer.size();	
	std::string::size_type const m_size = message.size();
	http_reply::buffer_type::size_type const new_b_size = b_size + m_size;
	buffer.resize(new_b_size);
	for (http_reply::buffer_type::size_type it = 0, _it = 0; 
		it < new_b_size && _it < m_size; 
		++it, ++_it)
	{
		buffer.at(it) = message.at(_it);
	}
}

static inline void cast_to_buffer(http_reply::status_type status, http_reply::buffer_type & buffer)
{
	switch (status) {
		case http_reply::ok:
			cast_helper(buffer, ok);
		break;
		case http_reply::created:
			cast_helper(buffer, created);
		break;
		case http_reply::accepted:
			cast_helper(buffer, accepted);
		break;
		case http_reply::no_content:
			cast_helper(buffer, no_content);
		break;
		case http_reply::partial_content:
			cast_helper(buffer, partial_content);
		break;
		case http_reply::multiple_choices:
			cast_helper(buffer, multiple_choices);
		break;
		case http_reply::moved_permanently:
			cast_helper(buffer, moved_permanently);
		break;
		case http_reply::moved_temporarily:
			cast_helper(buffer, moved_temporarily);
		break;
		case http_reply::not_modified:
			cast_helper(buffer, not_modified);
		break;
		case http_reply::bad_request:
			cast_helper(buffer, bad_request);
		break;
		case http_reply::unauthorized:
			cast_helper(buffer, unauthorized);
		break;
		case http_reply::forbidden:
			cast_helper(buffer, forbidden);
		break;
		case http_reply::not_found:
			cast_helper(buffer, not_found);
		break;
		case http_reply::internal_server_error:
			cast_helper(buffer, internal_server_error);
		break;
		case http_reply::not_implemented:
			cast_helper(buffer, not_implemented);
		break;
		case http_reply::bad_gateway:
			cast_helper(buffer, bad_gateway);
		break;
		case http_reply::service_unavailable:
			cast_helper(buffer, service_unavailable);	
		break;
		default:
			cast_helper(buffer, internal_server_error);
		break;
	} // switch(status)
} 

} // namespace status_strings

namespace misc_strings {

std::string const name_value_separator = ": ";
std::string const crlf = "\r\n";

} // namespace misc_strings

namespace stock_replies {

const char ok[] = "";
const char created[] =
  "<html>"
  "<head><title>Created</title></head>"
  "<body><h1>201 Created</h1></body>"
  "</html>";
const char accepted[] =
  "<html>"
  "<head><title>Accepted</title></head>"
  "<body><h1>202 Accepted</h1></body>"
  "</html>";
const char no_content[] =
  "<html>"
  "<head><title>No Content</title></head>"
  "<body><h1>204 Content</h1></body>"
  "</html>";
const char multiple_choices[] =
  "<html>"
  "<head><title>Multiple Choices</title></head>"
  "<body><h1>300 Multiple Choices</h1></body>"
  "</html>";
const char moved_permanently[] =
  "<html>"
  "<head><title>Moved Permanently</title></head>"
  "<body><h1>301 Moved Permanently</h1></body>"
  "</html>";
const char moved_temporarily[] =
  "<html>"
  "<head><title>Moved Temporarily</title></head>"
  "<body><h1>302 Moved Temporarily</h1></body>"
  "</html>";
const char not_modified[] =
  "<html>"
  "<head><title>Not Modified</title></head>"
  "<body><h1>304 Not Modified</h1></body>"
  "</html>";
const char bad_request[] =
  "<html>"
  "<head><title>Bad Request</title></head>"
  "<body><h1>400 Bad Request</h1></body>"
  "</html>";
const char unauthorized[] =
  "<html>"
  "<head><title>Unauthorized</title></head>"
  "<body><h1>401 Unauthorized</h1></body>"
  "</html>";
const char forbidden[] =
  "<html>"
  "<head><title>Forbidden</title></head>"
  "<body><h1>403 Forbidden</h1></body>"
  "</html>";
const char not_found[] =
  "<html>"
  "<head><title>Not Found</title></head>"
  "<body><h1>404 Not Found</h1></body>"
  "</html>";
const char internal_server_error[] =
  "<html>"
  "<head><title>Internal Server Error</title></head>"
  "<body><h1>500 Internal Server Error</h1></body>"
  "</html>";
const char not_implemented[] =
  "<html>"
  "<head><title>Not Implemented</title></head>"
  "<body><h1>501 Not Implemented</h1></body>"
  "</html>";
const char bad_gateway[] =
  "<html>"
  "<head><title>Bad Gateway</title></head>"
  "<body><h1>502 Bad Gateway</h1></body>"
  "</html>";
const char service_unavailable[] =
  "<html>"
  "<head><title>Service Unavailable</title></head>"
  "<body><h1>503 Service Unavailable</h1></body>"
  "</html>";

static inline std::string cast_to_string(http_reply::status_type status)
{
  switch (status)
  {
  case http_reply::ok:
    return ok;
  case http_reply::created:
    return created;
  case http_reply::accepted:
    return accepted;
  case http_reply::no_content:
    return no_content;
  case http_reply::multiple_choices:
    return multiple_choices;
  case http_reply::moved_permanently:
    return moved_permanently;
  case http_reply::moved_temporarily:
    return moved_temporarily;
  case http_reply::not_modified:
    return not_modified;
  case http_reply::bad_request:
    return bad_request;
  case http_reply::unauthorized:
    return unauthorized;
  case http_reply::forbidden:
    return forbidden;
  case http_reply::not_found:
    return not_found;
  case http_reply::internal_server_error:
    return internal_server_error;
  case http_reply::not_implemented:
    return not_implemented;
  case http_reply::bad_gateway:
    return bad_gateway;
  case http_reply::service_unavailable:
    return service_unavailable;
  default:
    return internal_server_error;
  }
  return internal_server_error;
}

} // namespace stock_replies

} // utility

#undef MAKE_CONST_STRING

#endif

