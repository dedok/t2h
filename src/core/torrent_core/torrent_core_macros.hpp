#ifndef TORRENT_CORE_MACROS_HPP_INCLUDED
#define TORRENT_CORE_MACROS_HPP_INCLUDED

#include "syslogger.hpp"
#include "core_version.hpp"

#if defined(__GNUG__)
#	pragma GCC system_header
#endif

#include <cstdio>
#include <libtorrent/session.hpp>

#define FUNCTION_PREFIX __FUNCTION__
#define TORRENT_CORE_LOG_PREFIX "TORRENT CORE"

#define TCORE_LOG_MAX_MESSAGE_SIZE SYS_LOGGER_MAX_MESSAGE_SIZE

#define TCORE_LOG_GENERIC(log_type, ...)																		\
do {																											\
	char * vat_ = new char[TCORE_LOG_MAX_MESSAGE_SIZE];															\
	std::memset(vat_, '\0', TCORE_LOG_MAX_MESSAGE_SIZE);														\
	std::sprintf(vat_, __VA_ARGS__);																			\
	log_type("%s [%s] %s %s", TORRENT_CORE_LOG_PREFIX, CORE_VERSION_STRING, FUNCTION_PREFIX, vat_)				\
	delete vat_; vat_ = NULL;																					\
} while(0); 

#if defined (T2H_DEBUG)
#	define TCORE_TRACE(...) \
		TCORE_LOG_GENERIC(LOG_TRACE, __VA_ARGS__)
#else
#	define TCORE_TRACE(...) { /* */ }
#endif

#define TCORE_WARNING(...) \
	TCORE_LOG_GENERIC(LOG_WARNING, __VA_ARGS__)

#define TCORE_ERROR(...) \
	TCORE_LOG_GENERIC(LOG_ERROR, __VA_ARGS__)

#define LIBTORRENT_EXCEPTION_SAFE_BEGIN 	\
try											\
{ 

#define LIBTORRENT_EXCEPTION_SAFE_END_(x)					\
} 															\
	catch (libtorrent::libtorrent_exception const & expt)	\
{															\
	TCORE_WARNING("Exteption caught '%s'", expt.what())		\
	x;														\
}

#define LIBTORRENT_EXCEPTION_SAFE_END LIBTORRENT_EXCEPTION_SAFE_END_(;)

#define TORRENT_DETAIL_BEGIN namespace t2h_core { namespace detail {

#define TORRENT_DETAIL_END } } // namesace t2h_core, detail

#endif

