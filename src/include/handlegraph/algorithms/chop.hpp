#ifndef HANDLEGRAPH_ALGORITHMS_CHOP_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_CHOP_HPP_INCLUDED

#include "handlegraph/mutable_path_deletable_handle_graph.hpp"

#include <functional>
#include <utility>
#include <vector>

namespace handlegraph {
namespace algorithms {

/**
 * Chop the graph so nodes are at most max_node_length. Preserves relative
 * ordering of nodes, but may reassign IDs. Preserves local forward orientation
 * of new pieces.
 *
 * Throws std::runtime_error if max_node_length is 0.
 *
 * Invalidates handles into the graph.
 */
void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length);

/**
 * Chop the graph so nodes are at most max_node_length. Preserves relative
 * ordering of nodes, but may reassign IDs. Preserves local forward orientation
 * of new pieces.
 *
 * Throws std::runtime_error if max_node_length is 0.
 *
 * Invalidates handles into the graph.
 *
 * Call the given callback, if any nodes change ID or are divided, to describe
 * where each new node starts on each old node ID. Passes (old node ID, start
 * offset along old node forward strand, same for reverse strand, new node
 * handle).
 *
 * During the callback, the new node will exist in the graph, while the old
 * node may not.
 */
void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length, const std::function<void(nid_t, size_t, size_t, handle_t)>& record_change);

/**
 * Chop the graph at the given points. Each chop point is a node ID and a list
 * of offsets along the forward strand of that node, where the node should be
 * divided. Preserves relative ordering of nodes, but may reassign IDs.
 * Preserves local forward orientation of new pieces.
 *
 * Node IDs that do not exist in the graph are silently ignored, as are offsets
 * that are not strictly between 0 and the length of the node. The same node
 * may be listed multiple times; all offsets are combined, and duplicate offsets
 * are ignored.
 *
 * Invalidates handles into the graph.
 */
void chop(MutablePathDeletableHandleGraph& graph, const std::vector<std::pair<nid_t, std::vector<size_t>>>& chop_points);

/**
 * Chop the graph at the given points. Each chop point is a node ID and a list
 * of offsets along the forward strand of that node, where the node should be
 * divided. Preserves relative ordering of nodes, but may reassign IDs.
 * Preserves local forward orientation of new pieces.
 *
 * Node IDs that do not exist in the graph are silently ignored, as are offsets
 * that are not strictly between 0 and the length of the node. The same node
 * may be listed multiple times; all offsets are combined, and duplicate offsets
 * are ignored.
 *
 * Invalidates handles into the graph.
 *
 * Call the given callback, if any nodes change ID or are divided, to describe
 * where each new node starts on each old node ID. Passes (old node ID, start
 * offset along old node forward strand, same for reverse strand, new node
 * handle).
 *
 * During the callback, the new node will exist in the graph, while the old
 * node may not.
 */
void chop(MutablePathDeletableHandleGraph& graph, const std::vector<std::pair<nid_t, std::vector<size_t>>>& chop_points, const std::function<void(nid_t, size_t, size_t, handle_t)>& record_change);

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
