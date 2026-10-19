#ifndef HANDLEGRAPH_ALGORITHMS_PATH_LENGTH_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_PATH_LENGTH_HPP_INCLUDED

/**
 * \file path_length.hpp
 *
 * Defines an algorithm for measuring the length of a path
 */

#include "handlegraph/path_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Returns the length of a path in bases. Uses the graph's own path length if it
/// is a PathPositionHandleGraph, and otherwise adds up the lengths of the visited nodes.
size_t get_path_length(const PathHandleGraph* graph, const path_handle_t& path);

}
}

#endif
