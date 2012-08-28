if (DEFINED T2H_CORE_SHARED)
	set(link_depends core)
else()
	set(link_depends common core ${libtorrent_LIBS} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

	# Setting manager test
	add_executable(setting_manager_test EXCLUDE_FROM_ALL setting_manager_test.cpp)
	target_link_libraries(setting_manager_test ${link_depends})

	# t2h_torrent_core_test
	add_executable(torrent_core_test torrent_core_test.cpp)
	target_link_libraries(torrent_core_test ${link_depends})
endif()

# Cpp/C linking test
add_executable(link_test_c EXCLUDE_FROM_ALL link_test.c)
target_link_libraries(link_test_c ${link_depends})

add_executable(link_test_cpp EXCLUDE_FROM_ALL link_test.cpp)
target_link_libraries(link_test_cpp core)


