# Http server application
add_executable(http_server EXCLUDE_FROM_ALL http_server.cpp)
target_link_libraries(http_server common core ${Boost_LIBRARIES})

# Full model application
add_executable(full_model EXCLUDE_FROM_ALL full_model.cpp)
target_link_libraries(full_model core common ${Boost_LIBRARIES})

