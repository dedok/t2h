# Logger tests
add_executable(logger_test EXCLUDE_FROM_ALL syslog_test.cpp)
target_link_libraries(logger_test common ${Boost_LIBRARIES})

# Http reply test
add_executable(http_reply_test EXCLUDE_FROM_ALL http_reply_test.cpp)
target_link_libraries(http_reply_test common ${Boost_LIBRARIES})

# Notification center test
add_executable(nc_test EXCLUDE_FROM_ALL notification_center_test.cpp)
target_link_libraries(nc_test common ${Boost_LIBRARIES})

add_executable(hhp_test EXCLUDE_FROM_ALL http_header_parser_test.cpp)
target_link_libraries(hhp_test common ${Boost_LIBRARIES})

