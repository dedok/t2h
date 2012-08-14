#
# CMake toolchain file for cmake build system, this toolchain 
# Containt settings for cross MacOS X compile
#
# and optionals arguments :
#	 -DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo
#	 -DSDK_VERSION:STRING=VERSION
#

include(CMakeForceCompiler)

# Set cmake system valiables
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(APPLE TRUE CACHE BOOL "" FORCE)

if (DEFINED ${SDK_VERSION})
	set(CMAKE_SYSTEM_VERSION ${SDK_VERSION})
else()
	set(CMAKE_SYSTEM_VERSION 10.6.5)
endif()

cmake_force_c_compiler("/usr/bin/gcc" GNU)
cmake_force_cxx_compiler("/usr/bin/g++" GNU)

# Test and set Debug or no Debug gcc/g++ options
if (DEFINED ${CMAKE_BUILD_TYPE})
	if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
		set(BUILD_TYPE_GCC_ARGS "-ggdb")
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
		set(BUILD_TYPE_GCC_ARGS "-fast -DNDEBUG")
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		set(BUILD_TYPE_GCC_ARGS "-g -fast -DNDEBUG")
	endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)
else()
	set(BUILD_TYPE_GCC_ARGS "")
endif()

# Set Mac OS X compiler flags
set(BASIC_COMPILER_FLAGS "-Wextra -fvisibility=hidden -arch x86_64 -arch i386 ${BUILD_TYPE_GCC_ARGS}")
set(CMAKE_C_FLAGS "${BASIC_COMPILER_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${BASIC_COMPILER_FLAGS} -fvisibility-inlines-hidden" CACHE STRING "" FORCE)
set(EXTRA_LINK_FLAGS "-macosx_version_min=${CMAKE_SYSTEM_VERSION}" CACHE STRING "" FORCE)	

