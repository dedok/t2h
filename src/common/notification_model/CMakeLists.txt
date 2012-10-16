set(COMMON_HEADERS ${COMMON_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/notification_center.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/base_notification.hpp          
	${CMAKE_CURRENT_SOURCE_DIR}/notification_receiver.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/notification_unit.hpp
	PARENT_SCOPE)

set(COMMON_SOURCES ${COMMON_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/notification_center.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/notification_unit.cpp
	PARENT_SCOPE)

