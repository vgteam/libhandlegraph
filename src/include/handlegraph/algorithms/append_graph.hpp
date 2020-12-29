#ifndef HANDLEGRAPH_ALGORITHMS_APPEND_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_APPEND_GRAPH_HPP_INCLUDED

/**
 * \file append_graph.hpp
 *
 * Defines algorithms for appending handle graphs
 */

#include "handlegraph/mutable_path_mutable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Append the heads of from to the tails of into
void append_handle_graph(const HandleGraph* from, MutableHandleGraph* into);
    
/// Append the heads of from to the tails of into
/// Append all (shared) paths of from to into, copy the rest.
/// If only_connect_path_tips is true, then only edges linking appended paths will be added
/// (as opposed to every head and tail)
void append_path_handle_graph(const PathHandleGraph* from, MutablePathMutableHandleGraph* into,
                              bool only_connect_path_tips = false);

}
}

#endif
