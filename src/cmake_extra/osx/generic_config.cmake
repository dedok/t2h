#
# Setup envt.
# Soshnikov Vasiliy <dedok.mad@gmail.com>
#

message(STATUS "Windows extra confgiguration")

# add libtorrent windows definitions
#if (CMAKE_BUILD_TYPE MATCHES Debug)
#	add_definitions(-DTORRENT_DEBUG)
#endif()

add_definitions(
	-DTORRENT_DISABLE_DHT 
	-DTORRENT_USE_OPENSSL 
	-DTORRENT_DISABLE_GEO_IP 
	-DUNICODE 
	-D_UNICODE 
	-DTORRENT_NO_DEPRECATE 
	-D_FILE_OFFSET_BITS=64
)
