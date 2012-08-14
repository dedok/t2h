function(copy_headers TARGET)
	foreach(HDR ${ARGN})
		copy_file(${HDR} ${INCLUDE_DIR})
		list(APPEND COMMON_HDRS ${INCLUDE_DIR}/${FILE_NAME})
	endforeach(HDR)
	add_custom_target(${TARGET}_headers DEPENDS ${COMMON_HDRS})
	add_dependencies(${TARGET} ${TARGET}_headers)
endfunction(copy_headers)

macro(copy_file FROM TO)
	get_filename_component(FILE_NAME ${FROM} NAME)
	add_custom_command(
		OUTPUT ${TO}/${FILE_NAME}
		COMMAND ${CMAKE_COMMAND} -E copy ${FROM} ${TO}/${FILE_NAME}
		MAIN_DEPENDENCY ${FROM}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endmacro(copy_file)

macro(add_subdirectories)
	foreach(DIR ${ARGV})
		add_subdirectory(${DIR})
	endforeach(DIR)
endmacro(add_subdirectories)

macro(prepare_source_list SRC_LIST_OUTPUT)
	foreach(DIR ${ARGN})
		foreach(SRC ${${DIR}_SRCS})
			list(APPEND ${SRC_LIST_OUTPUT} ${DIR}/${SRC})
		endforeach(SRC)
	endforeach(DIR)
endmacro(prepare_source_list)

macro(prepare_header_list HDR_LIST_OUTPUT)
	foreach(DIR ${ARGN})
		foreach(HDR ${${DIR}_COMMON_HDRS})
			list(APPEND ${HDR_LIST_OUTPUT} ${DIR}/${HDR})
		endforeach(HDR)
	endforeach(DIR)
endmacro(prepare_header_list)

macro(set_parent VAR)
	set(${VAR} ${ARGN})
	set(${VAR} ${ARGN} PARENT_SCOPE)
endmacro(set_parent)

##
# \def include_subsys(subsys dir...)
#
# Adds the specified subsystem root directory and maybe some its subdirectories
# to the current list of directories searched by a compiler for include files.
# 
# \param subsys the subsystem searched for directories
# \param dir... one or more subdirectories
# 
macro(include_subsys subsys)
	include_directories(${CMAKE_SOURCE_DIR}/libs/${subsys})
	foreach(dir ${ARGN})
		include_directories(${CMAKE_SOURCE_DIR}/libs/${subsys}/${dir})
	endforeach(dir)
endmacro(include_subsys)

##
# \def append(varname value)
#
# Appends a value to the end of specified variable.
#
# \param varname name of the variable to be changed
# \param value value to be added
#
macro(append varname value)
    set(${varname} ${${varname}}${value})
endmacro(append)

##
# \def add_cmake_build_action(name output script DEPENDS dependencies...)
#
# Creates CMake-based custom command which generates file in the current
# source directory using CMake and custom script; also creates custom target
# as an alias for that custom command (workaround for CMake sub-scripts).
#
# \param name name of target
# \param output output file name relative to the current source dir
# \param script name of CMake script file
# \param dependencies... optional dependencies (in addition to the local CMakeLists.txt)
#
macro(add_cmake_build_action name output script DEPENDS)
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${output}"
		COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR="${CMAKE_SOURCE_DIR}" -P ${script}
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" ${ARGN}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
	add_custom_target(
		${name}
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${output}"
	)
endmacro(add_cmake_build_action)
