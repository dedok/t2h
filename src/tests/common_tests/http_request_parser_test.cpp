#include "http_request_parser.hpp"

#include <iostream>
#include <boost/tuple/tuple.hpp>

std::string const request =
	"GET /Z7KQtifdfDL/Deathwolf.mp3 HTTP/1.0\r\n"
	"Host: 127.0.0.1:5999\r\n"
	"User-Agent: VLC/2.0.3 LibVLC/2.0.3\r\n"
	"Icy-MetaData: 1\r\n\r\n";

static inline boost::tribool parse_recv(utility::http_request & req, std::string const & req_) 
{
	boost::tribool result;
	utility::http_request_parser request_parser;
	boost::tie(result, boost::tuples::ignore) = request_parser.parse(req, req_.begin(), req_.end());
	return result;
}

static inline void simple_test() 
{
	using namespace utility;
	http_request client_request;

	boost::tribool result = parse_recv(client_request, request);
	if (result) {
		std::cerr << "Good " << std::endl;
		return;
	}
	else if (!result) {
		std::cerr << "Bad " << std::endl;
		return;
	}
	std::cerr << "more data" << std::endl;
}

int main() 
{
	simple_test();	
	return 0;
}

