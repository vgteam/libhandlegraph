#ifndef HANDLEGRAPH_ALGORITHMS_UNCHOP_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_UNCHOP_HPP_INCLUDED

#include "handlegraph/mutable_path_deletable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/**
 * Unchop by gluing abutting handles with just a single edge between them and
 * compatible path steps together.
 */
void unchop(MutablePathDeletableHandleGraph* graph);

/**
 * Chop the graph so nodes are at most max_node_length
 */
void chop(MutableHandleGraph* graph, size_t max_node_length);

}
}

#endif
