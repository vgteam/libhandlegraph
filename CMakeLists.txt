# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.1)

# Demand old SWIG target naming behavior. New behavior is only available on
# CMake 3.13+, so we can't use it.
cmake_policy(SET CMP0078 OLD)
# And this one starts happening on CMake 3.14
cmake_policy(SET CMP0086 OLD)

# This defines default install directories like "lib"
include(GNUInstallDirs)

# Set a default install directory.
# See https://stackoverflow.com/a/39485990/402891
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr CACHE PATH FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Project's name
project(libhandlegraph)
# We build using c++14
set(CMAKE_CXX_STANDARD 14)

# Set up to use SWIG to generate Python bindings
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Find the Python development files
find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

# Use all standard-compliant optimizations
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

# Let cmake decide where to put the output files, allowing for out-of-tree builds.

# The following folder will be included
include_directories("src/include")

# List all the .cpp files, which need to go into the normal libraries and the
# SWIG library
set(SRCS
  src/types.cpp
  src/handle_graph.cpp
  src/path_handle_graph.cpp
  src/empty_handle_graph.cpp
  )

add_library(handlegraph_objs OBJECT
  ${SRCS}
  src/include/handlegraph/handle_graph.hpp
  src/include/handlegraph/mutable_handle_graph.hpp
  src/include/handlegraph/deletable_handle_graph.hpp
  src/include/handlegraph/path_handle_graph.hpp
  src/include/handlegraph/mutable_path_handle_graph.hpp
  src/include/handlegraph/mutable_path_mutable_handle_graph.hpp
  src/include/handlegraph/mutable_path_deletable_handle_graph.hpp
  src/include/handlegraph/util.hpp
  src/include/handlegraph/types.hpp
  src/include/handlegraph/iteratee.hpp
  )

# Build objects position-independent to allow a shared library
set_property(TARGET handlegraph_objs PROPERTY POSITION_INDEPENDENT_CODE 1)

# Set up SWIG
SET(CMAKE_SWIG_FLAGS "")

set_source_files_properties(src/bindings.i PROPERTIES CPLUSPLUS ON)
set_property(SOURCE src/bindings.i PROPERTY SWIG_MODULE_NAME handlegraph)
swig_add_module(handlegraph python
  src/bindings.i
  ${SRCS}
  )
swig_link_libraries(handlegraph ${PYTHON_LIBRARIES})

# Make static and shared versions with the same base name
add_library(handlegraph_shared SHARED $<TARGET_OBJECTS:handlegraph_objs>)
set_target_properties(handlegraph_shared PROPERTIES OUTPUT_NAME handlegraph)
add_library(handlegraph_static STATIC $<TARGET_OBJECTS:handlegraph_objs>)
set_target_properties(handlegraph_static PROPERTIES OUTPUT_NAME handlegraph)

# Set up for installability
install(TARGETS handlegraph_shared handlegraph_static 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY src/include/handlegraph
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  FILES_MATCHING PATTERN "*.hpp"
  )
