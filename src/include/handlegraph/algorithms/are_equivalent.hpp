#ifndef HANDLEGRAPH_ALGORITHMS_ARE_EQUIVALENT_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_ARE_EQUIVALENT_HPP_INCLUDED

/**
 * \file are_equivalent.hpp
 *
 * Defines an algorithm for determining whether two graphs are identical
 */

#include "handlegraph/path_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Checks whether two graphs are identical in their IDs, sequences, and edges.
/// Optionally reports why graphs are found non-equivalent to stderr.
bool are_equivalent(const HandleGraph* graph_1,
                    const HandleGraph* graph_2, bool verbose = false);

/// Checks whether two graphs are identical in their IDs, sequences, edges, and paths.
/// Optionally reports why graphs are found non-equivalent to stderr.
bool are_equivalent_with_paths(const PathHandleGraph* graph_1,
                               const PathHandleGraph* graph_2, bool verbose = false);
}
}

#endif
