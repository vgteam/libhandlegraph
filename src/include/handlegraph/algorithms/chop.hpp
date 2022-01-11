#ifndef HANDLEGRAPH_ALGORITHMS_CHOP_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_CHOP_HPP_INCLUDED

#include "handlegraph/mutable_path_deletable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/**
 * Chop the graph so nodes are at most max_node_length. Preserves relative
 * ordering of nodes, but may reassign IDs. Preserves local forward orientation
 * of new pieces.
 *
 * Invalidates handles into the graph.
 */
void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length);

/**
 * Chop the graph so nodes are at most max_node_length. Preserves relative
 * ordering of nodes, but may reassign IDs. Preserves local forward orientation
 * of new pieces.
 *
 * Invalidates handles into the graph.
 *
 * Call the given callback, if any nodes change ID or are divided, to describe
 * where each new node ID starts on each old node ID. Passes (old node ID,
 * start offset along old node, new node ID).
 */
void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length, const std::function<void(nid_t, size_t, nid_t)>& record_change);

/**
 * Unchop by gluing abutting handles with just a single edge between them and
 * compatible path steps together. Broadly preserves relative ordering of
 * nodes.
 *
 * Invalidates handles into the graph.
 */
void unchop(MutablePathDeletableHandleGraph& graph);

}
}

#endif
