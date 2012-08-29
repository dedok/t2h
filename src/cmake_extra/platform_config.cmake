#
# Find and setup boost lib under Mac OS X.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#

message(STATUS "Setup extra depends for ${PLATFORM_TYPE}")
include(${CMAKE_SOURCE_DIR}/cmake_extra/${PLATFORM_TYPE}/config.cmake OPTIONAL)

# Try to find/setup the boost library
add_definitions(-DTORRENT_USE_OPENSSL) 
add_definitions(-DTORRENT_DISABLE_GEO_IP) 
add_definitions(-DBOOST_ASIO_ENABLE_CANCELIO)

if (DEFINED T2H_CORE_SHARED)
	add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)
else()
	add_definitions(-DBOOST_ASIO_DYN_LINK)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DATE_TIME ON)
set(Boost_USE_STATIC_RUNTIME ${use_static_runtime})

find_package(Boost 1.45.0 COMPONENTS
	filesystem
	program_options
	thread
	system
	date_time
	signals
	iostreams
	random
	unit_test_framework
	REQUIRED)

if (UNIX OR APPLE)
	set(Boost_LIBRARIES ${Boost_LIBRARIES} pthread)
	set(Boost_THREAD_LIBRARY ${Boost_THREAD_LIBRARY} pthread)
endif()

# OpenSSL
set(OPENSSL_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_RUNTIME ${use_static_runtime})

find_package(OpenSSL PATHS ${CMAKE_SOURCE_DIR}/cmake_extra REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIR})
include_directories(${libtorrent_INCLUDE_DIR})

