#
# Common cmake entry.
# At this part of build process we setting up compiper,
# extra variables, extra libraries, test envt.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#

cmake_minimum_required(VERSION 2.8)

# Add build configurations
if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES
		Debug Release RelWithDebInfo
		CACHE STRING "Supported build types" FORCE
	)
endif(CMAKE_CONFIGURATION_TYPES)

project(torrent2http)

# Add version to scope
include(cmake_extra/version.cmake)

# Set build option to default value(only if we does not set the build type early)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

# Define the more convenient variable with more convenient values
set(CPU_TYPE "stub" CACHE INTERNAL "" FORCE)
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL AMD64)
	set(CPU_TYPE "x64" CACHE INTERNAL "" FORCE)
else()
	set(CPU_TYPE "x86" CACHE INTERNAL "" FORCE)
endif()

# Detect and define platform type
set(PLATFORM_TYPE "stub" CACHE INTERNAL "" FORCE)
if(APPLE)
	set(PLATFORM_TYPE "osx" CACHE INTERNAL "" FORCE)
elseif(WIN32)
	set(PLATFORM_TYPE "windows" CACHE INTERNAL "" FORCE)
elseif(UNIX)
	set(PLATFORM_TYPE "unix" CACHE INTERNAL "" FORCE)
endif()

# This file should contain all additional include and link directories
set(EXTRA_CONF_DIR ${CMAKE_SOURCE_DIR}/cmake_extra/${PLATFORM_TYPE})
include(${EXTRA_CONF_DIR}/platform_config.cmake)

# Specifying directory for executable and libraries files with 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../lib)
set(PACKAGES_DIR ${CMAKE_SOURCE_DIR}/../packages)

# 3rd-party
include_directories(${PACKAGES_DIR}/include)

# Exported ours
set(INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
include_directories(${INCLUDE_DIR})

# Add subroutines(eg copy_headers ...)
include(${CMAKE_SOURCE_DIR}/cmake_extra/subroutines.cmake)

# Add sub directories(eg sources, libs, scripts ...)
add_subdirectory(core)
add_subdirectory(common)

add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(scripts)

