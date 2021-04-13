# $Id$

cmake_minimum_required(VERSION 3.14)

#set(CMAKE_VERBOSE_MAKEFILE ON)

set(PLATFORM mips-openwrt-linux-musl)
set(OPENWRT_DIR  /work/projects/openwrt/openwrt-18.06.9)
set(STAGING_DIR ${OPENWRT_DIR}/git/staging_dir)
set(SET_ENV env STAGING_DIR=${STAGING_DIR})
set(CMAKE_CXX_COMPILER ${SET_ENV} ${STAGING_DIR}/toolchain-mips_24kc_gcc-7.3.0_musl/bin/${PLATFORM}-g++)
set(CMAKE_STRIP ${SET_ENV} ${STAGING_DIR}/toolchain-mips_24kc_gcc-7.3.0_musl/bin/${PLATFORM}-strip)


set(Qt5_DIR ${OPENWRT_DIR}/sysroot/opt/Qt-mips-5.15.2-release/lib/cmake/Qt5)
set(QT_DIR  ${Qt5_DIR})

set(GIT_HEADER ".git.h")

#set(ENV{STAGING_DIR} ${STAGING_DIR})
#message("STAGING_DIR=[$ENV{STAGING_DIR}]")

#link_directories(/usr/${PLATFORM}/lib /lib/${PLATFORM})

set(PROJECT_NAME "tst")
project(${PROJECT_NAME} LANGUAGES CXX)
set(PROJECT_UNSTRIPPED ${PROJECT_NAME}-unstripped)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES *.cpp)
file(GLOB HEADERS include/*.h)

add_executable(${PROJECT_UNSTRIPPED} #EXCLUDE_FROM_ALL
  ${SOURCES}
  ${HEADERS}
)

target_link_libraries(${PROJECT_UNSTRIPPED}
    Qt${QT_VERSION_MAJOR}::Core
)

add_custom_command(
    TARGET ${PROJECT_UNSTRIPPED} POST_BUILD
    COMMAND ${CMAKE_BUILD_TOOL} ${PROJECT_NAME}
)

add_custom_target(strip
    DEPENDS ${PROJECT_NAME}
)

add_custom_target(${PROJECT_NAME}
    DEPENDS ${PROJECT_UNSTRIPPED}
    COMMAND ${CMAKE_STRIP} ${PROJECT_UNSTRIPPED} -o ${PROJECT_NAME}
    COMMENT "Strip ${PROJECT_NAME}"
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_BUILD_TOOL} install
)

install(
    DIRECTORY DESTINATION ${OPENWRT_DIR}/nfs
)

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/${PROJECT_NAME}
    DESTINATION ${OPENWRT_DIR}/nfs
)

set_property(
    TARGET ${PROJECT_NAME}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME} ${GIT_HEADER}
)

add_custom_target(distclean
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/scripts/distclean.cmake
)

add_custom_target(${GIT_HEADER}
    COMMAND [ -e ${GIT_HEADER} ] && grep VERSION ${GIT_HEADER} | cut -f3- -d' ' || sh ${CMAKE_SOURCE_DIR}/scripts/git.sh
)
add_dependencies( ${PROJECT_UNSTRIPPED} ${GIT_HEADER} )

add_custom_target(git
    COMMAND rm -f ${GIT_HEADER}
    COMMAND ${CMAKE_BUILD_TOOL} ${GIT_HEADER}
)

add_custom_target(gitupd
    COMMAND ${CMAKE_BUILD_TOOL} git
    COMMAND ${CMAKE_BUILD_TOOL} all
)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})
