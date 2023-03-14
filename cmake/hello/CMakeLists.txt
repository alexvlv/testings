# $Id$

cmake_minimum_required(VERSION 3.14)

#set(CMAKE_VERBOSE_MAKEFILE ON)


#set(GIT_HEADER ".git.h")

#set(ENV{STAGING_DIR} ${STAGING_DIR})
#message("STAGING_DIR=[$ENV{STAGING_DIR}]")

#link_directories(/usr/${PLATFORM}/lib /lib/${PLATFORM})

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

add_executable(${PROJECT_UNSTRIPPED} #EXCLUDE_FROM_ALL
  ${SOURCES}
  ${HEADERS}
)

