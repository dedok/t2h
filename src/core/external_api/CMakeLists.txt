set(T2H_HEADERS 
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/handles_manager.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/external_api_details.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_config.h
	${CMAKE_CURRENT_SOURCE_DIR}/t2h.h
	${CMAKE_CURRENT_SOURCE_DIR}/hc_event_source_adapter.hpp
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/external_api.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/external_api_details.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hc_event_source_adapter.cpp
	PARENT_SCOPE
	)

