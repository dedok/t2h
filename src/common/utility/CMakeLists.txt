set(COMMON_HEADERS ${COMMON_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/factory.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/common_utility.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/misc_utility.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread_pool.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/events_policy.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/events.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/event_wrapper.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/basic_events.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/basic_safe_container.hpp
	PARENT_SCOPE)

set(COMMON_SOURCES ${COMMON_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/misc_utility.cpp
	PARENT_SCOPE)

