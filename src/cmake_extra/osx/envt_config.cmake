#
# Setup envt.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#
message(STATUS "Setup envt. for OSX paltform")

# Test envt. definitions
if (NOT DEFINED LIBTORRENT_ROOT OR NOT DEFINED BOOST_ROOT)
	message(FATAL_ERROR "Configure error : LIBTORRENT_ROOT ot BOOST_ROOT not set")
endif()

if (NOT EXISTS ${LIBTORRENT_ROOT} OR NOT EXISTS ${BOOST_ROOT})  
	message(FATAL_ERROR "Configure error : can not find ${LIBTORRENT_ROOT} or ${BOOST_ROOT}")
endif()

set(libtorrent_INCLUDE_DIR ${LIBTORRENT_ROOT}/include)
set(libtorrent_LIBS ${LIBTORRENT_ROOT}/libtorrent-rasterbar.a)

if (NOT EXISTS ${libtorrent_INCLUDE_DIR})
	message(FATAL_ERROR "Configure error : can not find ${libtorrent_INCLUDE_DIR}")
endif()

if (NOT EXISTS ${libtorrent_LIBS})
	message(FATAL_ERROR "Configure error : can not find ${libtorrent_LIBS}")
endif()
