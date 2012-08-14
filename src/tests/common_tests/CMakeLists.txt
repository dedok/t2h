# Logger tests
add_executable(logger_test EXCLUDE_FROM_ALL syslog_test.cpp)
target_link_libraries(logger_test common ${Boost_LIBRARIES})

# Http reply test
add_executable(http_reply_test EXCLUDE_FROM_ALL http_reply_test.cpp)
target_link_libraries(http_reply_test common core ${Boost_LIBRARIES})


