# Some tests might not link well only if 't2h' build as dynamic lib.
# That happens because not all 't2h' api(deep hidden classes) have export attributes.
if (T2H_CORE_SHARED)
	set(link_depends t2h_core ${Boost_LIBRARIES})
else()
	set(link_depends common t2h_core ${libtorrent_LIBS} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})	
	# Http server application
	add_executable(http_server EXCLUDE_FROM_ALL http_server.cpp)
	target_link_libraries(http_server common t2h_core ${link_depends})
endif()

# Full model application
add_executable(full_model EXCLUDE_FROM_ALL full_model.cpp)
target_link_libraries(full_model ${link_depends})

