#include "handlegraph/mutable_path_deletable_handle_graph.hpp"

/** \file mutable_path_deletable_handle_graph.cpp
 * Implement methods for MutablePathDeletableHandleGraph.
 */

namespace handlegraph {

handle_t MutablePathDeletableHandleGraph::change_sequence(const handle_t& handle,
                                                          const std::string& sequence) {
    // TODO: repetitive with DeletableHandleGraph, but we can't call it because it could destroy
    // all the paths containing the handle as a side effect of the destroy_handle call at the end
    
    // new handle with the new sequence
    handle_t new_handle = create_handle(sequence);
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
    
    // collect the steps we need to replace
    std::vector<step_handle_t> steps;
    for_each_step_on_handle(handle, [&](const step_handle_t& step) {
        steps.push_back(step);
    });
    
    // replace them
    std::vector<handle_t> rewriter(1, new_handle);
    for (const auto& step : steps) {
        rewrite_segment(step, get_next_step(step), rewriter);
    }
    // clear the original
    destroy_handle(handle);
    
    return new_handle;
}

}


