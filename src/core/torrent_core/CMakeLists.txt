set(DETAILS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/details)

set(T2H_HEADERS ${T2H_HEADERS}
# base
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/base_torrent_core_cntl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/sequential_torrent_controller.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_info.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_future.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_macros.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_event_handler.hpp
# details
	${DETAILS_PATH}/base_resolver.hpp
	${DETAILS_PATH}/error_resolvers_types.hpp
	${DETAILS_PATH}/resolvers_factory.hpp
	${DETAILS_PATH}/lookup_error.hpp	
	${DETAILS_PATH}/shared_buffer.hpp
	${DETAILS_PATH}/torrent_core_utility.hpp
	PARENT_SCOPE)
				   
set(T2H_SOURCES ${T2H_SOURCES}
# base
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sequential_torrent_controller.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_core_future.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/torrent_info.cpp
# details
	${DETAILS_PATH}/error_resolvers_types.cpp
	${DETAILS_PATH}/resolvers_factory.cpp
	${DETAILS_PATH}/lookup_error.cpp	
	${DETAILS_PATH}/shared_buffer.cpp
	${DETAILS_PATH}/torrent_core_utility.cpp
	PARENT_SCOPE)

