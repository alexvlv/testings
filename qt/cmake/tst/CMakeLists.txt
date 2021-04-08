# $Id$

set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.14)

project(tst LANGUAGES CXX)

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

add_executable(tst
  ${SOURCES}
  ${HEADERS}
)
target_link_libraries(tst Qt${QT_VERSION_MAJOR}::Core)
