#ifndef HANDLEGRAPH_MUTABLE_PATH_MUTABLE_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_MUTABLE_PATH_MUTABLE_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the MutablePathMutableHandleGraph interface for graphs that can have paths and graph material changed.
 */

#include "handlegraph/mutable_path_handle_graph.hpp"
#include "handlegraph/mutable_handle_graph.hpp"

namespace handlegraph {

/**
 * This is the interface for a graph which is mutable and which has paths which are also mutable.
 * Some additional semantics are assumed:
 * - divide_handle() replaces every occurrence of the original handle with its subsegments
 *   in all stesps on all paths
 * - apply_orientation() also applies the orientation to all occurrences of the handle
 *   in all paths
 */
using MutablePathMutableHandleGraph = HandleGraph<MutablePaths, Mutable>;

};

}

#endif
