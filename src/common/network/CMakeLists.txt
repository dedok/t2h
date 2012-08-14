set(USE_ASIO_MULTI_THREAD_TRANSPORT TRUE)

if (USE_ASIO_MULTI_THREAD_TRANSPORT)
	set(IMPL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
	set(IMPL_HEADERS 
		${IMPL_PATH}/connection.hpp
		${IMPL_PATH}/asio_socket_transport.hpp
		)
	set(IMPL_SOURCES
		${IMPL_PATH}/connection.cpp
		${IMPL_PATH}/asio_socket_transport.cpp
		)
endif()

set(COMMON_HEADERS
	${COMMON_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/base_transport_ev_handler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/base_transport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/transport_types.hpp
	${IMPL_HEADERS}
	PARENT_SCOPE
	)

set(COMMON_SOURCES 
	${COMMON_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/base_transport_ev_handler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base_transport.cpp
	${IMPL_SOURCES}
	PARENT_SCOPE
	)


