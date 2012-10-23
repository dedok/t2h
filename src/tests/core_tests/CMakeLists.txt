# Some tests might not link well only if 't2h' build as dynamic lib.
# That happens because not all 't2h' api(deep hidden classes) have export attributes.
if (T2H_CORE_SHARED)
	set(link_depends t2h_core ${Boost_LIBRARIES})
else()
	set(link_depends common t2h_core ${libtorrent_LIBS} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

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
target_link_libraries(link_test_cpp ${link_depends})

# Cpp/C type range test
add_executable(external_type_test EXCLUDE_FROM_ALL external_type_test.cpp)
target_link_libraries(external_type_test ${link_depends})

# http server replies test
add_executable(hc_replies_test EXCLUDE_FROM_ALL hc_replies_test.cpp)
target_link_libraries(hc_replies_test ${link_depends})

