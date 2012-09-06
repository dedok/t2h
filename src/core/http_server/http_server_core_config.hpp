#ifndef HTTP_SERVER_CORE_CONFIG_HPP_INCLUDED
#define HTTP_SERVER_CORE_CONFIG_HPP_INCLUDED

#include "syslogger.hpp"
#include "core_version.hpp"

#define HTTP_SERVER_LOG_PREFIX "HTTP SERVER CORE "

#if defined (T2H_DEBUG)
#	define HCORE_TRACE(...) \
		LOG_TRACE(HTTP_SERVER_LOG_PREFIX __VA_ARGS__)
#else
#	define HCORE_TRACE(...) { /* */ }
#endif

#define HCORE_WARNING(...) \
	LOG_WARNING(HTTP_SERVER_LOG_PREFIX __VA_ARGS__)

#define HCORE_ERROR(...) \
	LOG_ERROR(HTTP_SERVER_LOG_PREFIX __VA_ARGS__)

#endif 

