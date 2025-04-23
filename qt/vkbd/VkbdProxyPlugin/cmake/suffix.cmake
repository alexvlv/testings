# $Id$

if (CMAKE_CROSSCOMPILING)
	message("-- Cross compiling for architecture: ${CMAKE_SYSTEM_PROCESSOR}")
else()
	message("-- Native compiling for architecture: ${CMAKE_SYSTEM_PROCESSOR}")
	add_custom_command(
		TARGET  ${PROJECT_NAME} POST_BUILD
		COMMAND mv ${PROJECT_NAME} ${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}
	)
	set_property(
		TARGET ${PROJECT_NAME}
		APPEND
		PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}-${CMAKE_SYSTEM_PROCESSOR}
	)

	#set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX "-${CMAKE_SYSTEM_PROCESSOR}")
	#set_target_properties(${PROJECT_UNSTRIPPED} PROPERTIES SUFFIX "-${CMAKE_SYSTEM_PROCESSOR}")
endif()
