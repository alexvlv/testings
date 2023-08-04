# $Id$

set(cmake_generated
    ${CMAKE_BINARY_DIR}/CMakeCache.txt
    ${CMAKE_BINARY_DIR}/cmake_install.cmake
    ${CMAKE_BINARY_DIR}/Makefile
    ${CMAKE_BINARY_DIR}/CMakeFiles
    ${CMAKE_BINARY_DIR}/.cmake
    ${CMAKE_BINARY_DIR}/install_manifest.txt
    ${CMAKE_BINARY_DIR}/.git.h
	${CMAKE_BINARY_DIR}/.qt
)

foreach(file ${cmake_generated})
  if (EXISTS ${file})
     file(REMOVE_RECURSE ${file})
  endif()
endforeach(file)
