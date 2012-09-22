set(T2H_HEADERS 
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/handles_manager.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/external_api_details.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/t2h_config.h
	${CMAKE_CURRENT_SOURCE_DIR}/t2h.h
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/external_api.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/external_api_details.cpp
	PARENT_SCOPE
	)

