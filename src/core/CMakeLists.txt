set(TARGET core)

# Adding common sublibraries
add_subdirectory(external_api)
add_subdirectory(common)
add_subdirectory(http_server)
add_subdirectory(torrent_core)

set(T2H_HEADERS ${T2H_HEADERS})
set(T2H_SOURCES ${T2H_SOURCES})

# Create static or shared library
if (DEFINED T2H_CORE_SHARED)	
	message(STATUS "Shared build setup type is on")
	set(link_depends ${libtorrent_LIBS} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} common)
	add_library(${TARGET} SHARED ${T2H_SOURCES})
	add_dependencies(${TARGET} common)
	target_link_libraries(${TARGET} ${link_depends})
else()
	message(STATUS "Static build setup type is on")
	add_library(${TARGET} STATIC ${T2H_SOURCES}) 
	add_dependencies(${TARGET} common_headers ${NETWORK_HEADERS})
endif()

copy_headers(${TARGET} ${T2H_HEADERS})
