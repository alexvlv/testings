# $Id$

cmake_minimum_required(VERSION 3.14)
#set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(PLATFORM mips-openwrt-linux-musl)

set(OPENWRT_LNK ${CMAKE_CURRENT_SOURCE_DIR}/.openwrt)
execute_process(
	COMMAND readlink -s -e ${OPENWRT_LNK}
	OUTPUT_VARIABLE OPENWRT_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(STAGING_DIR ${OPENWRT_DIR}/git/staging_dir)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(CMAKE_CXX_COMPILER ${SET_ENV} ${STAGING_DIR}/toolchain-mips_24kc_gcc-7.3.0_musl/bin/${PLATFORM}-g++)
set(CMAKE_C_COMPILER ${SET_ENV} ${STAGING_DIR}/toolchain-mips_24kc_gcc-7.3.0_musl/bin/${PLATFORM}-gcc)
set(CMAKE_STRIP ${SET_ENV} ${STAGING_DIR}/toolchain-mips_24kc_gcc-7.3.0_musl/bin/${PLATFORM}-strip)

execute_process(
	COMMAND basename ${CMAKE_CURRENT_SOURCE_DIR}
	ERROR_QUIET
	OUTPUT_VARIABLE PROJECT_NAME
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("-- Configure project:[${PROJECT_NAME}]")
add_compile_definitions(APPNAME="${PROJECT_NAME}")

project(${PROJECT_NAME} LANGUAGES CXX C)
set(PROJECT_UNSTRIPPED ${PROJECT_NAME}-unstripped)

set(SYSROOT_DIR ${OPENWRT_DIR}/sysroot)
set(Qt5_DIR ${SYSROOT_DIR}/opt/Qt-mips-5.15.2-release/lib/cmake/Qt5)
set(QT_DIR  ${Qt5_DIR})
set(CMAKE_PREFIX_PATH ${Qt5_DIR})
find_package(QT NAMES Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES *.cpp )
file(GLOB HEADERS *.h )
file(GLOB FORMS *.ui )

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -Wno-sign-conversion -Wno-deprecated-declarations -Wno-unused-variable")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -pipe -mips32 -fpermissive  -DQT_NO_DEBUG -D_REENTRANT -fPIC")

set(CMAKE_SYSROOT ${SYSROOT_DIR})
set (CMAKE_FIND_ROOT_PATH ${SYSROOT_DIR})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set(CMAKE_BUILD_WITH_INSTALL_RPATH and CMAKE_INSTALL_RPATH before add_executable)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "/opt/Qt-mips-5.15.2-release/lib")

find_library(gthread gthread-2.0 HINTS ${SYSROOT_DIR}/usr/lib )
find_library(glib glib-2.0 HINTS ${SYSROOT_DIR}/usr/lib )

add_executable(${PROJECT_UNSTRIPPED} #EXCLUDE_FROM_ALL
  ${SOURCES}
  ${HEADERS}
  ${FORMS}
)

target_link_libraries(${PROJECT_UNSTRIPPED}
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Network
	gthread-2.0
	glib-2.0
)

add_custom_command(
    TARGET ${PROJECT_UNSTRIPPED} POST_BUILD
	COMMAND +${CMAKE_BUILD_TOOL} ${PROJECT_NAME}
)

add_custom_target(strip
    DEPENDS ${PROJECT_NAME}
)

add_custom_target(${PROJECT_NAME}
	DEPENDS ${PROJECT_UNSTRIPPED}
	COMMAND ${CMAKE_STRIP} ${PROJECT_UNSTRIPPED} -o ${PROJECT_NAME}
	COMMENT "Strip ${PROJECT_NAME}"
)

include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/gitversion.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/clean.cmake)

add_custom_target(rebuild
	COMMAND ${CMAKE_BUILD_TOOL} recmake
	COMMAND ${CMAKE_BUILD_TOOL} -j ${PROJECT_NAME}
)
