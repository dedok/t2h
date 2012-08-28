set(T2H_HEADERS
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/http_transport_ev_handler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core_config.hpp
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/http_transport_ev_handler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core.cpp
	PARENT_SCOPE
	)

