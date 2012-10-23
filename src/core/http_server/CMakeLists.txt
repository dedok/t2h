set(T2H_HEADERS
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/http_transport_ev_handler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/replies_types.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/partial_content_reply.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/head_reply.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/root_reply.hpp
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/http_transport_ev_handler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/partial_content_reply.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/head_reply.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/replies/root_reply.cpp
	PARENT_SCOPE
	)

