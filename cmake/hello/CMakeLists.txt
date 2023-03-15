# $Id$

cmake_minimum_required(VERSION 3.14)

#set(CMAKE_VERBOSE_MAKEFILE ON)

get_filename_component(APP_NAME ${CMAKE_SOURCE_DIR} NAME)
message("Target project name: [${APP_NAME}]")

set(PROJECT_NAME  ${APP_NAME})
project(${PROJECT_NAME} LANGUAGES CXX)
set(PROJECT_UNSTRIPPED ${PROJECT_NAME}-unstripped)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES *.cpp)
file(GLOB HEADERS include/*.h)

include(${CMAKE_SOURCE_DIR}/scripts/gitversion.cmake)

add_executable(${PROJECT_UNSTRIPPED}
  ${SOURCES}
  ${HEADERS}
)
add_dependencies( ${PROJECT_UNSTRIPPED} git )

set_property(
    TARGET ${PROJECT_UNSTRIPPED}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME} ${GIT_HEADER}
)

add_custom_target(strip
    DEPENDS ${PROJECT_NAME}
)

add_custom_target(${PROJECT_NAME}  ALL
    DEPENDS ${PROJECT_UNSTRIPPED}
    COMMAND ${CMAKE_STRIP} ${PROJECT_UNSTRIPPED} -o ${PROJECT_NAME}
    #COMMENT "Strip ${PROJECT_NAME}"
)

add_custom_target(distclean
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/scripts/distclean.cmake
)
