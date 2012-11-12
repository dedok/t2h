if (USE_MONGOOSE_HTTP_TRANSPORT)
	add_definitions(-DUSE_MONGOOSE_HTTP_TRANSPORT)
endif()

if (USE_ASIO_MULTI_THREAD_TRANSPORT)
	message(STATUS "Asio transport is deptecated since ...")
endif()


if (USE_MONGOOSE_HTTP_TRANSPORT)
	set(IMPL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/details)
	set(IMPL_SOURCES 
		${IMPL_PATH}/http_mongoose_transport.cpp 
		${IMPL_PATH}/mongoose.c
		)
	set (IMPL_HEADERS
		${IMPL_PATH}/http_mongoose_transport.hpp 
		${IMPL_PATH}/http_transport_context.hpp 
		${IMPL_PATH}/mongoose.h)
endif()

set(COMMON_HEADERS
	${COMMON_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/transport_types.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/transport_context.hpp
	${IMPL_HEADERS}
	PARENT_SCOPE
	)

set(COMMON_SOURCES 
	${COMMON_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/base_transport.cpp
	${IMPL_SOURCES}
	PARENT_SCOPE
	)

