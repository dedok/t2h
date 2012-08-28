configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/core_version.hpp_tmpl
	${CMAKE_CURRENT_SOURCE_DIR}/core_version.hpp
)

set(T2H_HEADERS 
	${T2H_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/setting_manager.hpp
	PARENT_SCOPE
	)

set(T2H_SOURCES 
	${T2H_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/setting_manager.cpp
	PARENT_SCOPE
	)

