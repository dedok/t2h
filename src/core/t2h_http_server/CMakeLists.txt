set(T2H_HEADERS
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_settings_manager.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_transport_ev_handler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_http_server_cntl.hpp
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_settings_manager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_transport_ev_handler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_http_server_cntl.cpp
	PARENT_SCOPE
	)

