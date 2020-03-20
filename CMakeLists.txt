# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.1)

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

# Use all standard-compliant optimizations
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

# Let cmake decide where to put the output files, allowing for out-of-tree builds.

# The following folder will be included
include_directories("src/include")

# Don't use @rpath in Mac builds' install_name fields (AKA LC_ID_DYLIB in otool
# -l output). Populate with an absolute path instead. This will let us actually
# find the library when we use it as a CMake external project and don't fully
# install it to any normal lib directory.
set (CMAKE_MACOSX_RPATH OFF)
# The install_name gets modified on installation to be this.
set (CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

add_library(handlegraph_objs OBJECT
  src/handle.cpp
  src/include/handlegraph/handle_graph.hpp
  src/include/handlegraph/mutable_handle_graph.hpp
  src/include/handlegraph/deletable_handle_graph.hpp
  src/include/handlegraph/path_handle_graph.hpp
  src/include/handlegraph/path_position_handle_graph.hpp
  src/include/handlegraph/mutable_path_handle_graph.hpp
  src/include/handlegraph/mutable_path_mutable_handle_graph.hpp
  src/include/handlegraph/mutable_path_deletable_handle_graph.hpp
  src/include/handlegraph/expanding_overlay_graph.hpp
  src/include/handlegraph/util.hpp
  src/include/handlegraph/types.hpp
  src/include/handlegraph/iteratee.hpp
  )

# Build objects position-independent to allow a shared library
set_property(TARGET handlegraph_objs PROPERTY POSITION_INDEPENDENT_CODE 1)

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
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
  )
