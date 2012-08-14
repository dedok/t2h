# Setting manager test
add_executable(setting_manager_test EXCLUDE_FROM_ALL setting_manager_test.cpp)
target_link_libraries(setting_manager_test common core ${Boost_LIBRARIES})

#Cpp/C linking test
add_executable(t2h_link_test_c EXCLUDE_FROM_ALL link_test.c)
target_link_libraries(t2h_link_test_c common core ${Boost_LIBRARIES})

add_executable(t2h_link_test_cpp EXCLUDE_FROM_ALL link_test.cpp)
target_link_libraries(t2h_link_test_cpp common core ${Boost_LIBRARIES})



