set(link_depends ${libtorrent_LIBS} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

# Http server application
add_executable(http_server EXCLUDE_FROM_ALL http_server.cpp)
target_link_libraries(http_server common core ${link_depends})

# Full model application
add_executable(full_model EXCLUDE_FROM_ALL full_model.cpp)
target_link_libraries(full_model core common ${link_depends})

