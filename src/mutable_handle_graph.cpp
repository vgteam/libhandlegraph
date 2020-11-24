#include "handlegraph/mutable_handle_graph.hpp"

/** \file mutable_handle_graph.cpp
 * Implement MutableHandleGraph methods
 */

namespace handlegraph {

void MutableHandleGraph::increment_node_ids(nid_t increment) {
    // Increment IDs by just reassigning IDs and applying the increment as the ID translation
    reassign_node_ids([&](const nid_t& old_id) -> nid_t {
        return old_id + increment;
    });
}

void MutableHandleGraph::increment_node_ids(long increment) {
    increment_node_ids((nid_t)increment);
}

}


