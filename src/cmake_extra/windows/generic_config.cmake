#
# Setup envt.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#

message(STATUS "Windows extra confgiguration")

# add windows generic definitions
add_definitions(
	-D_WIN32_WINDOWS
	-D_CRT_SECURE_NO_WARNINGS
	-D_SCL_SECURE_NO_WARNINGS
	-DWIN32_LEAN_AND_MEAN 
	-D_WIN32_WINNT=0x0500
	-D_UNICODE
	-D__USE_W32_SOCKETS 
	/Zc:wchar_t 
	/Zc:forScope 
	/MP 
	-D_FILE_OFFSET_BITS=64
)

# add libtorrent windows definitions
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DTORRENT_DEBUG)
endif()

add_definitions(
	-DTORRENT_LOGGING
	-DTORRENT_STATS
	-DTORRENT_DISABLE_ENCRYPTION
	-DTORRENT_DISABLE_GEO_IP 
	-DTORRENT_DISABLE_INVARIANT_CHECKS
	-DTORRENT_NO_DEPRECATE
	-DTORRENT_USE_PERFORMANCE_TIMER=1
)
	
