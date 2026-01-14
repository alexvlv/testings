# $Id$

add_custom_target(distclean
	COMMAND ${CMAKE_BUILD_TOOL} clean
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/distclean.cmake
)

add_custom_target(recmake
	COMMAND ${CMAKE_BUILD_TOOL} distclean
	COMMAND $(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
)
