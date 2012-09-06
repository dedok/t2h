#ifndef TORRENT_CORE_MACROS_HPP_INCLUDED
#define TORRENT_CORE_MACROS_HPP_INCLUDED

#include "syslogger.hpp"
#include <libtorrent/session.hpp>

#define TORRENT_CORE_LOG_PREFIX "TORRENT CORE "
#define FUNCTION_PREFIX __FUNCTION__" "

#define TCORE_TRACE(...) \
	LOG_TRACE(TORRENT_CORE_LOG_PREFIX __VA_ARGS__)

#define TCORE_WARNING(...) \
	LOG_WARNING(TORRENT_CORE_LOG_PREFIX __VA_ARGS__)

#define TCORE_ERROR(...) \
	LOG_ERROR(TORRENT_CORE_LOG_PREFIX __VA_ARGS__)

#define LIBTORRENT_EXCEPTION_SAFE_BEGIN \
try	\
{ 

#define LIBTORRENT_EXCEPTION_SAFE_END_(x)					\
} 															\
	catch (libtorrent::libtorrent_exception const & expt)	\
{															\
	x;														\
}

#define LIBTORRENT_EXCEPTION_SAFE_END LIBTORRENT_EXCEPTION_SAFE_END_(;)

#endif

