#include "handlegraph/mutable_path_handle_graph.hpp"

/** \file mutable_path_handle_graph.cpp
 * Implement MutablePathHandleGraph methods
 */

namespace handlegraph {

void MutablePathHandleGraph::destroy_paths(const std::vector<path_handle_t>& paths) {
    for (const auto& path : paths) {
        destroy_path(path);
    }
}

void MutablePathHandleGraph::pop_front_step(const path_handle_t& path_handle) {
    step_handle_t begin = path_begin(path_handle);
    step_handle_t next = get_next_step(begin);
    rewrite_segment(begin, next, std::vector<handle_t>());
}

void MutablePathHandleGraph::pop_back_step(const path_handle_t& path_handle) {
    step_handle_t last = path_back(path_handle);
    step_handle_t prev = get_previous_step(last);
    rewrite_segment(prev, last, std::vector<handle_t>());
}

path_handle_t MutablePathHandleGraph::rename_path(const path_handle_t& path_handle,
                                                  const std::string& new_name) {
    if (new_name == get_path_name(path_handle)) {
        return path_handle;
    }
    // if no overwritten implementation, just copy the path
    path_handle_t renamed = create_path_handle(new_name);
    for (handle_t handle : scan_path(path_handle)) {
        append_step(renamed, handle);
    }
    set_circularity(renamed, get_is_circular(path_handle));
    destroy_path(path_handle);
    return renamed;
}

}


