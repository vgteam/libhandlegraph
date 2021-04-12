#ifndef HANDLEGRAPH_ALGORITHMS_CHOP_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_CHOP_HPP_INCLUDED

#include "handlegraph/mutable_path_deletable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/**
 * Chop the graph so nodes are at most max_node_length. Preserves relative
 * ordering of node IDs.
 */
void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length);

/**
 * Unchop by gluing abutting handles with just a single edge between them and
 * compatible path steps together. Preserves relative ordering of node IDs.
 */
void unchop(MutablePathDeletableHandleGraph& graph);

}
}

#endif
