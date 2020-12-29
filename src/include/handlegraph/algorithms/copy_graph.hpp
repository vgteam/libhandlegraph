#ifndef HANDLEGRAPH_ALGORITHMS_COPY_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_COPY_GRAPH_HPP_INCLUDED

/**
 * \file copy_graph.hpp
 *
 * Defines algorithms for copying data between handle graphs
 */

#include "handlegraph/mutable_path_mutable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Copies the nodes and edges from one graph into another. If 'into'
/// is non-empty, nodes in 'from' will replace nodes that have the same
/// ID.
void copy_handle_graph(const HandleGraph* from, MutableHandleGraph* into);

/// Copies the nodes, edges, and paths from one graph into another.If 'into'
/// is non-empty, nodes and paths in 'from' will replace nodes that have the same
/// ID and paths that have the same name.
void copy_path_handle_graph(const PathHandleGraph* from, MutablePathMutableHandleGraph* into);

/// Copies a path from one graph to another. Nodes and edges to support
/// the path must already exist.
void copy_path(const PathHandleGraph* from, const path_handle_t& path,
               MutablePathHandleGraph* into);

}
}

#endif
