set(COMMON_HEADERS 
	${COMMON_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/http_header.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_request_parser.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_request.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_reply.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_reply_resources.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/mime_types.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_utility.hpp
	PARENT_SCOPE
	)

set(COMMON_SOURCES 
	${COMMON_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/http_request.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_reply.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_request_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mime_types.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_utility.cpp
	PARENT_SCOPE
	)

