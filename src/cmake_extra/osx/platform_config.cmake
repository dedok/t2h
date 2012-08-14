#
# Find and setup boost lib under Mac OS X.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#

message(STATUS "Setup extra depends, for OSX platform.")
include(${CMAKE_SOURCE_DIR}/cmake_extra/osx/lib_dir.cmake OPTIONAL)

# Try to find the boost library
add_definitions(-DBOOST_ALL_NO_LIB)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DATE_TIME ON)
set(Boost_USE_STATIC_RUNTIME ${use_static_runtime})

find_package(Boost 1.44.0 COMPONENTS
	filesystem
	program_options
	thread
	system
	date_time
	signals
	iostreams
	unit_test_framework
	REQUIRED)

set(Boost_LIBRARIES ${Boost_LIBRARIES} pthread)
set(Boost_THREAD_LIBRARY ${Boost_THREAD_LIBRARY} pthread)

include_directories(${Boost_INCLUDE_DIR})

