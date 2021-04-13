# $Id$

cmake_minimum_required(VERSION 3.0.0)

message(STATUS "Resolving GIT Version")

set(_build_version "unknown")

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _git_hash
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} status --porcelain
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _git_star
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(_build_version ${_git_hash}${_git_star})


  message( STATUS "GIT ver: ${_build_version}")
else()
  message(STATUS "GIT not found")
endif()

string(TIMESTAMP _time_stamp)

configure_file(${local_dir}/scripts/gitversion.h.in ${output_dir}/gitversion.h @ONLY)

# https://bravenewmethod.com/2017/07/14/git-revision-as-compiler-definition-in-build-with-cmake/
# https://github.com/tikonen/blog/tree/master/cmake/git_version
