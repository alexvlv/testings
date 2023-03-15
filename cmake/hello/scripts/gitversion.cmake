# $Id$

cmake_minimum_required(VERSION 3.0.0)

set(_build_version "unknown")
set(_git_hash "")
set(_git_branch "")
set(_git_date "")

set(GIT_HEADER ".git.h")

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_hash
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} status --porcelain
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_star
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list HEAD
        COMMAND wc -l
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_cnt
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
	if(NOT _git_star STREQUAL "")
		set(_git_star "*")
	endif()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_branch
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(_git_date_fmt "format:%Y-%m-%d %H:%M")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -n1 --format=%cd --date=${_git_date_fmt}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_date
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(_build_version r${_git_cnt}-${_git_hash}${_git_star})
    message( STATUS "GIT: ${_build_version} on ${_git_branch} at ${_git_date}")
else()
    message(STATUS "GIT not found")
endif()

string(TIMESTAMP _build_timestamp "%Y-%m-%d %H:%M" )

configure_file(${CMAKE_CURRENT_LIST_DIR}/gitversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/${GIT_HEADER} @ONLY)

add_custom_target(clean_git
    COMMAND rm -f ${GIT_HEADER}
)

add_custom_target(git
    #COMMAND ${CMAKE_BUILD_TOOL} clean_git
    #COMMAND ${CMAKE_BUILD_TOOL} cmake
    COMMAND rm -f ${GIT_HEADER}
    COMMAND $(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
)

add_custom_target(cmake
    COMMAND $(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})

# https://bravenewmethod.com/2017/07/14/git-revision-as-compiler-definition-in-build-with-cmake/
# https://github.com/tikonen/blog/tree/master/cmake/git_version

# COMMAND bash "-c" "echo -n hello | sed 's/hello/world/;'"
# https://stackoverflow.com/questions/35689501/cmakes-execute-process-and-arbitrary-shell-scripts

