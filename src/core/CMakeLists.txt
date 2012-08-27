set(TARGET core)

# Adding common sublibraries
add_subdirectory(external_api)
add_subdirectory(common)
add_subdirectory(http_server)
add_subdirectory(torrent_core)

set(T2H_HEADERS ${T2H_HEADERS})
set(T2H_SOURCES ${T2H_SOURCES})

# Create static library
if (T2H_CORE_SHARED)
	add_library(${TARGET} SHARED ${T2H_SOURCES}) 
else()
	add_library(${TARGET} STATIC ${T2H_SOURCES}) 
endif()

add_dependencies(${TARGET} common_headers ${NETWORK_HEADERS})
copy_headers(${TARGET} ${T2H_HEADERS})
