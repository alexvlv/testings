cmake_minimum_required(VERSION 3.16)
#set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

execute_process(
	COMMAND basename ${CMAKE_CURRENT_SOURCE_DIR}
	ERROR_QUIET
	OUTPUT_VARIABLE PROJECT_NAME
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("-- Configure project:[${PROJECT_NAME}]")
add_compile_definitions(APPNAME="${PROJECT_NAME}")

#set(ENV{PATH} "/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/bin:/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/sbin:/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/bin:/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/sbin:/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/bin/../x86_64-pokysdk-linux/bin:/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux:/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux-musl:$ENV{PATH}") 
#set(ENV{OECORE_TARGET_SYSROOT} /tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/armv8a-poky-linux)
#set(ENV{PKG_CONFIG_SYSROOT_DIR} $ENV{OECORE_TARGET_SYSROOT})
#set(ENV{PKG_CONFIG_PATH} $ENV{OECORE_TARGET_SYSROOT}/usr/lib/pkgconfig:$ENV{OECORE_TARGET_SYSROOT}/usr/share/pkgconfig)

#include(/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/share/cmake/OEToolchainConfig.cmake)
#set(CMAKE_TOOLCHAIN_FILE /work/tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/share/cmake/armv8a-poky-linux-toolchain.cmake)
#set(CMAKE_TOOLCHAIN_FILE /tools/fsl-imx-xwayland/5.15.71-2.2.0-kirkstone/sysroots/x86_64-pokysdk-linux/usr/share/cmake/OEToolchainConfig.cmake)

#set(Qt6_DIR ${SYSROOT_DIR}/usr/lib/cmake/Qt6)
#set(QT_DIR  ${Qt6_DIR})

project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX)
set(PROJECT_UNSTRIPPED ${PROJECT_NAME}-unstripped)
set(PROJECT_NAME_BIN ${PROJECT_NAME})

if( ${PROJECT_NAME_BIN} STREQUAL ${PROJECT_NAME})
	set(PROJECT_UNSTRIPPED_BIN ${PROJECT_UNSTRIPPED})
else()
	set(PROJECT_UNSTRIPPED_BIN lib${PROJECT_UNSTRIPPED}.so)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND QT_PACKAGES
	Widgets
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_PACKAGES})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_PACKAGES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB HEADERS *.h )
file(GLOB SOURCES *.cpp )
file(GLOB FORMS *.ui )
file(GLOB RCC *.qrc )

set(PROJECT_SOURCES
	${HEADERS}
	${SOURCES}
	${FORMS}
	${RCC}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(${PROJECT_UNSTRIPPED}
		MANUAL_FINALIZATION
		${PROJECT_SOURCES}
		Dialog.h Dialog.cpp Dialog.ui
	)
else()
	add_executable(${PROJECT_UNSTRIPPED}
		${PROJECT_SOURCES}
	)
endif()

set(QT_LIBS ${QT_PACKAGES})
list(TRANSFORM QT_LIBS PREPEND Qt${QT_VERSION_MAJOR}::)
target_link_libraries(${PROJECT_UNSTRIPPED} PRIVATE
	${QT_LIBS}
)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(${PROJECT_UNSTRIPPED})
endif()

add_custom_command(
	TARGET ${PROJECT_UNSTRIPPED} POST_BUILD
	COMMAND +${CMAKE_BUILD_TOOL} ${PROJECT_NAME}
)

add_custom_target(strip
	DEPENDS ${PROJECT_NAME}
)

add_custom_target(${PROJECT_NAME}
	DEPENDS ${PROJECT_UNSTRIPPED}
	COMMAND ${CMAKE_STRIP} ${PROJECT_UNSTRIPPED_BIN} -o ${PROJECT_NAME_BIN}
	COMMENT "Strip ${PROJECT_NAME}"
)

if(NOT ${PROJECT_NAME_BIN} STREQUAL ${PROJECT_NAME})
set_property(
	TARGET ${PROJECT_NAME}
	APPEND
	PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME_BIN} ${PROJECT_UNSTRIPPED_BIN}
)
endif()

add_custom_target(gitupd
	COMMAND +${CMAKE_BUILD_TOOL} git
	COMMAND +${CMAKE_BUILD_TOOL} all
)

include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/gitversion.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/clean.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/suffix.cmake)
