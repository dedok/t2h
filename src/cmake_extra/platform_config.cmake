#
# Setup evnt. libs. for current platform
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#
message(STATUS "Setup extra depends for ${PLATFORM_TYPE}")

# T2H library definitions
if (DEFINED T2H_CORE_SHARED)
	add_definitions(-DT2H_EXPORT)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug) 
	add_definitions(-DT2H_DEBUG)
endif()

if(DEFINED T2H_INT_WORKAROUND)
	message(STATUS "Enable int workaraund")
	add_definitions(-DT2H_INT_WORKAROUND)
endif()

# Generic_config allow to setup envt. for the libraries, envt_config allow to setup paths to libraries
include(${CMAKE_SOURCE_DIR}/cmake_extra/${PLATFORM_TYPE}/generic_config.cmake OPTIONAL)
include(${CMAKE_SOURCE_DIR}/cmake_extra/${PLATFORM_TYPE}/envt_config.cmake OPTIONAL)

# Find & setup the boost library
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DATE_TIME ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# Setup Boost library.
# Allow to avoid link errors inside Boost.Asio, if we link against boost dynamicly.
# Also add to link abainst t2h platform libraries as part of Boost link rule.
add_definitions(-DBOOST_ASIO_ENABLE_CANCELIO -DBOOST_DISABLE_EXCEPTION -DBOOST_ASIO_SEPARATE_COMPILATION)

find_package(Boost 1.45.0 COMPONENTS
	filesystem
	program_options
	thread
	system
	date_time
	signals
	iostreams
	random
	regex
	unit_test_framework
	REQUIRED)

if (UNIX OR APPLE)
	set(Boost_LIBRARIES ${Boost_LIBRARIES} pthread)
	set(Boost_THREAD_LIBRARY ${Boost_THREAD_LIBRARY} pthread)
endif()

if (WIN32)
	set(Boost_LIBRARIES ${Boost_LIBRARIES} wsock32 ws2_32) 
endif()

# Setup OpenSSL
set(OPENSSL_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_RUNTIME OFF)

find_package(OpenSSL PATHS ${CMAKE_SOURCE_DIR}/cmake_extra REQUIRED)

# Add to compiler flags extra library headers
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${libtorrent_INCLUDE_DIR})

