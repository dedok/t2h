#
# Setup envt.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#
message(STATUS "Setup envt. for Windows paltform")

# Test envt. definitions
if (NOT DEFINED LIBTORRENT_ROOT OR NOT DEFINED BOOST_ROOT)
	message(FATAL_ERROR "Configure error : LIBTORRENT_ROOT ot BOOST_ROOT not set")
endif()

if (NOT EXISTS ${LIBTORRENT_ROOT} OR NOT EXISTS ${BOOST_ROOT})  
	message(FATAL_ERROR "Configure error : can not find ${LIBTORRENT_ROOT} or ${BOOST_ROOT}")
endif()

# Switch between debug/release version of the libtorrent.
# When test libtorrent paths.
set(libtorrent_INCLUDE_DIR ${LIBTORRENT_ROOT}/include)
if (CMAKE_BUILD_TYPE MATCHES Debug)
	set(libtorrent_LIBS ${LIBTORRENT_ROOT}/bin/lib/libtorrentd.lib)
else()
	set(libtorrent_LIBS ${LIBTORRENT_ROOT}/bin/lib/libtorrent.lib)
endif()

if (NOT EXISTS ${libtorrent_INCLUDE_DIR})
	message(FATAL_ERROR "Configure error : can not find ${libtorrent_INCLUDE_DIR}")
endif()

if (NOT EXISTS ${libtorrent_LIBS})
	message(FATAL_ERROR "Configure error : can not find ${libtorrent_LIBS}")
endif()
