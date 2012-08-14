set(TARGET core)

# Adding common sublibraries
add_subdirectory(t2h_c_api)
add_subdirectory(t2h_common)
add_subdirectory(t2h_http_server)
add_subdirectory(t2h_torrent_core)

set(T2H_HEADERS ${T2H_HEADERS})
set(T2H_SOURCES ${T2H_SOURCES})

# Create static library
add_library(${TARGET} STATIC ${T2H_SOURCES}) 
add_dependencies(${TARGET} common_headers ${NETWORK_HEADERS})
copy_headers(${TARGET} ${T2H_HEADERS})
