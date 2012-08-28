#ifndef TORRENT_CORE_MACROS_HPP_INCLUDED
#define TORRENT_CORE_MACROS_HPP_INCLUDED

#include <libtorrent/session.hpp>

#define LIBTORRENT_EXCEPTION_SAFE_BEGIN \
try { 

#define LIBTORRENT_EXCEPTION_SAFE_END_(x)					\
} catch (libtorrent::libtorrent_exception const & expt) {	\
	x;														\
}

#define LIBTORRENT_EXCEPTION_SAFE_END LIBTORRENT_EXCEPTION_SAFE_END_(;)

#endif

