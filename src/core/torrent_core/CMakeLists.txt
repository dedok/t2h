set(T2H_HEADERS ${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/base_torrent_core_cntl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/serial_download_tcore_cntl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_info.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_future.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_macros.hpp
	PARENT_SCOPE)
				   
set(T2H_SOURCES ${T2H_SOURCES} 
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/serial_download_tcore_cntl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_future.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_info.cpp
	PARENT_SCOPE)

