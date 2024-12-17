#include "handlegraph/deletable_handle_graph.hpp"

/** \file deletable_handle_graph.cpp
 * Implement methods for DeletableHandleGraph.
 */

namespace handlegraph {

handle_t DeletableHandleGraph::truncate_handle(const handle_t& handle, bool trunc_left, size_t offset) {
    auto halves = divide_handle(handle, offset);
    if (trunc_left) {
        destroy_handle(halves.first);
        return halves.second;
    }
    else {
        destroy_handle(halves.second);
        return halves.first;
    }
}

handle_t DeletableHandleGraph::change_sequence(const handle_t& handle, const std::string& sequence) {
    // new handle with the new sequence
    handle_t new_handle = create_handle(sequence);
    if (get_is_reverse(handle)) {
        new_handle = flip(new_handle);
    }
    
    // copy its edges
    follow_edges(handle, false, [&](const handle_t& next) {
        create_edge(new_handle, next);
    });
    follow_edges(handle, true, [&](const handle_t& prev) {
        // ensure that we don't double add a non-reversing self-edge
        if (get_id(prev) != get_id(handle) || get_is_reverse(prev)) {
            create_edge(prev, new_handle);
        }
    });
    
    // clear the original
    destroy_handle(handle);
    
    return new_handle;
}

}


