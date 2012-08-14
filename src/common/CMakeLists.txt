# Target name, coz we waiting a library as the result of the work, 
# the name of output file shall have follow prefix 'lib' and the extension '.a/.lib'
set(TARGET common)

# Adding common sublibraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/network)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utility)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/syslogger)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/event_queue)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/http_utility)

set(COMMON_HEADERS ${COMMON_HEADERS} base_service.hpp services_manager.hpp)
set(COMMON_SOURCES ${COMMON_SOURCES} base_service.cpp services_manager.cpp)

# Create static library 
add_library(${TARGET} STATIC ${COMMON_SOURCES}) 
copy_headers(${TARGET} ${COMMON_HEADERS})

