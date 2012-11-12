#ifndef LC_LOGGER_CONFIG_HPP_INCLUDED
#define LC_LOGGER_CONFIG_HPP_INCLUDE

#include "syslogger.hpp"

#if defined(__GNUG__)
#	pragma GCC system_header
#endif

#define LIBCOMMON_PREFIX "LIBCOMMON"
#define LCFUNCTION_PREFIX __FUNCTION__

#define LC_LOG_MAX_MESSAGE_SIZE SYS_LOGGER_MAX_MESSAGE_SIZE

#define LC_LOG_GENERIC(log_type, ...)																			\
do {																											\
	char * vat_ = new char[LC_LOG_MAX_MESSAGE_SIZE];															\
	std::memset(vat_, '\0', LC_LOG_MAX_MESSAGE_SIZE);															\
	std::sprintf(vat_, __VA_ARGS__);																			\
	log_type("%s %s %s", LIBCOMMON_PREFIX, LCFUNCTION_PREFIX, vat_)												\
	delete vat_; vat_ = NULL;																					\
} while(0); 

#if defined (T2H_DEBUG)
#	define LC_TRACE(...) \
		LC_LOG_GENERIC(LOG_TRACE, __VA_ARGS__)
#else
#	define LC_TRACE(...) { /* */ }
#endif // T2H_DEBUG

#define LC_WARNING(...) \
	LC_LOG_GENERIC(LOG_WARNING, __VA_ARGS__)

#define LC_ERROR(...) \
	LC_GENERIC(LOG_ERROR, __VA_ARGS__)

#endif 

