# Http server application
add_executable(http_server EXCLUDE_FROM_ALL http_server.cpp)
target_link_libraries(http_server common core ${Boost_LIBRARIES})

