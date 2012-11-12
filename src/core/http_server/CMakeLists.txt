set(T2H_HEADERS
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/details/file_info_buffer.hpp	
	${CMAKE_CURRENT_SOURCE_DIR}/details/file_info_buffer_realtime_updater.hpp
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/http_server_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/details/file_info_buffer.cpp	
	${CMAKE_CURRENT_SOURCE_DIR}/details/file_info_buffer_realtime_updater.cpp
	PARENT_SCOPE
	)

