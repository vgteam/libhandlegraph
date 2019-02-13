# Specify the minimum version for CMake

cmake_minimum_required(VERSION 3.1)

# Set a default install directory.
# See https://stackoverflow.com/a/39485990/402891
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr CACHE PATH FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Project's name
project(libhandlegraph)
# We build using c++14
set(CMAKE_CXX_STANDARD 14)

# Use all standard-compliant optimizations
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

# Let cmake decide where to put the output files, allowing for out-of-tree builds.

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}")

add_library(handlegraph
  ${CMAKE_SOURCE_DIR}/src/handle.cpp
  ${CMAKE_SOURCE_DIR}/src/handle.hpp
  ${CMAKE_SOURCE_DIR}/src/handle_helper.hpp
  ${CMAKE_SOURCE_DIR}/src/handle_types.hpp
  )

if (APPLE)
elseif (TRUE)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Set up for installability
install(TARGETS handlegraph
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES 
  ${CMAKE_SOURCE_DIR}/src/handle.hpp
  ${CMAKE_SOURCE_DIR}/src/handle_helper.hpp
  ${CMAKE_SOURCE_DIR}/src/handle_types.hpp
  DESTINATION include)
