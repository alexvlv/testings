# $Id$

if(EXISTS ${CMAKE_SOURCE_DIR}/.qwt-lib )
	execute_process(
		COMMAND readlink "${CMAKE_SOURCE_DIR}/.qwt-lib"
		OUTPUT_VARIABLE QWT_LIB_DIR
		ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	message( "-- Qwt library directory: ${QWT_LIB_DIR}" )
	execute_process(
		COMMAND readlink "${CMAKE_SOURCE_DIR}/.qwt-src"
		OUTPUT_VARIABLE QWT_SRC_DIR
		ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	message( "-- Qwt source directory: ${QWT_SRC_DIR}" )

	list(APPEND QWT_INCLUDE_DIRS "${QWT_SRC_DIR}/classincludes")
	list(APPEND QWT_INCLUDE_DIRS "${QWT_SRC_DIR}/src")

	add_library(qwt SHARED IMPORTED)
	set_target_properties(qwt PROPERTIES
	  IMPORTED_LOCATION "${QWT_LIB_DIR}/libqwt.so"
	  INTERFACE_INCLUDE_DIRECTORIES  "${QWT_INCLUDE_DIRS}"
	)

	target_link_libraries(${PROJECT_UNSTRIPPED}
		qwt
	)

	# Old way:
	#include_directories(${QWT_SRC_DIR}/classincludes)
	#include_directories(${QWT_SRC_DIR}/src)
	# link_directories before add_executable!
	#link_directories(${QWT_LIB_DIR}/ )
endif()
