set(link_depends ${libtorrent_LIBS} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

# Setting manager test
add_executable(setting_manager_test EXCLUDE_FROM_ALL setting_manager_test.cpp)
target_link_libraries(setting_manager_test common core ${link_depends})

# Cpp/C linking test
add_executable(link_test_c EXCLUDE_FROM_ALL link_test.c)
target_link_libraries(link_test_c common core ${link_depends})

add_executable(link_test_cpp EXCLUDE_FROM_ALL link_test.cpp)
target_link_libraries(link_test_cpp common core ${link_depends})

# t2h_torrent_core_test
add_executable(tor_test tor_test.cpp)
target_link_libraries(tor_test common core ${link_depends})

