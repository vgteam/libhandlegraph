# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# This defines default install directories like "lib"
include(GNUInstallDirs)

# Project's name
project(libhandlegraph)
# We build using c++14
set(CMAKE_CXX_STANDARD 14)

# Use all standard-compliant optimizations
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

# Let cmake decide where to put the output files, allowing for out-of-tree builds.

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # We are probably an external project. Don't use @rpath in Mac builds'
    # install_name fields (AKA LC_ID_DYLIB in otool -l output). Populate with
    # an absolute path instead. This will let us actually find the library when
    # we use it as a CMake external project and don't fully install it to any
    # normal lib directory.
    message("libhandlegraph is root project or external_project")
    set (CMAKE_MACOSX_RPATH OFF)
else()
    # We are probably an add_subdirectory. We will expect to be in the root
    # project's lib directory, so we do want to have our non-installed
    # install_name use @rpath.
    message("libhandlegraph is add_subdirectory project")
    set (CMAKE_MACOSX_RPATH ON)
endif()

# The install_name gets modified on installation to be this.
set (CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

add_library(handlegraph_objs OBJECT
  src/deletable_handle_graph.cpp
  src/handle_graph.cpp
  src/mutable_handle_graph.cpp
  src/path_metadata.cpp
  src/mutable_path_metadata.cpp 
  src/path_handle_graph.cpp 
  src/path_position_handle_graph.cpp
  src/mutable_path_handle_graph.cpp
  src/ranked_handle_graph.cpp
  src/serializable.cpp
  src/snarl_decomposition.cpp
  src/trivially_serializable.cpp
  src/types.cpp
  src/copy_graph.cpp
  src/append_graph.cpp
  src/are_equivalent.cpp
  src/find_tips.cpp
  src/topological_sort.cpp
  src/apply_orientations.cpp
  src/is_single_stranded.cpp
  src/count_walks.cpp
  src/eades_algorithm.cpp
  src/dagify.cpp
  src/strongly_connected_components.cpp
  src/find_shortest_paths.cpp
  src/dijkstra.cpp
  src/is_acyclic.cpp
  src/reverse_complement.cpp
  src/split_strands.cpp
  src/chop.cpp
  src/weakly_connected_components.cpp
  src/extend.cpp
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
  src/include/handlegraph/algorithms/copy_graph.hpp
  src/include/handlegraph/algorithms/append_graph.hpp
  src/include/handlegraph/algorithms/are_equivalent.hpp
  src/include/handlegraph/algorithms/find_tips.hpp
  src/include/handlegraph/algorithms/topological_sort.hpp
  src/include/handlegraph/algorithms/apply_orientations.hpp
  src/include/handlegraph/algorithms/is_single_stranded.hpp
  src/include/handlegraph/algorithms/count_walks.hpp
  src/include/handlegraph/algorithms/eades_algorithm.hpp
  src/include/handlegraph/algorithms/dagify.hpp
  src/include/handlegraph/algorithms/strongly_connected_components.hpp
  src/include/handlegraph/algorithms/find_shortest_paths.hpp
  src/include/handlegraph/algorithms/dijkstra.hpp
  src/include/handlegraph/algorithms/reverse_complement.hpp
  src/include/handlegraph/algorithms/is_acyclic.hpp
  src/include/handlegraph/algorithms/split_strands.hpp
  src/include/handlegraph/algorithms/chop.hpp
  src/include/handlegraph/algorithms/weakly_connected_components.hpp
  src/include/handlegraph/algorithms/extend.hpp
  )

# Use the include directory when building the objects.
# It can't be picked up via dependency by the other libraries even if it's public.
target_include_directories(handlegraph_objs PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

# Build objects position-independent to allow a shared library
set_target_properties(handlegraph_objs PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# Make static and shared versions with the same base name.
# Make sure to give them interface include directories that depending targets can use.
add_library(handlegraph_shared SHARED $<TARGET_OBJECTS:handlegraph_objs>)
set_target_properties(handlegraph_shared PROPERTIES OUTPUT_NAME handlegraph)
target_include_directories(handlegraph_shared INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
add_library(handlegraph_static STATIC $<TARGET_OBJECTS:handlegraph_objs>)
set_target_properties(handlegraph_static PROPERTIES OUTPUT_NAME handlegraph)
target_include_directories(handlegraph_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

# Set up for installability
install(TARGETS handlegraph_shared handlegraph_static 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY src/include/handlegraph
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
  )
