#include "handlegraph/path_handle_graph.hpp"
#include "handlegraph/util.hpp"

#include <vector>

/** \file path_handle_graph.cpp
 * Implement PathHandleGraph and associated utilities' methods
 */

namespace handlegraph {

std::vector<step_handle_t> PathHandleGraph::steps_of_handle(const handle_t& handle,
                                                            bool match_orientation) const {
    std::vector<step_handle_t> found;
    
    for_each_step_on_handle(handle, [&](const step_handle_t& step) {
        // For each handle step
        if (!match_orientation || get_is_reverse(handle) == get_is_reverse(get_handle_of_step(step))) {
            // If its orientation is acceptable, keep it
            found.push_back(step);
        }
    });
    
    return found;
}

size_t PathHandleGraph::get_step_count(const handle_t& handle) const {
    size_t count = 0;
    for_each_step_on_handle(handle, [&](const step_handle_t& step) {
        ++count;
    });
    return count;
}

bool PathHandleGraph::is_empty(const path_handle_t& path_handle) const {
    // By default, we can answer emptiness queries with the length query.
    // But some implementations may have an expensive length query and a cheaper emptiness one
    return get_step_count(path_handle) == 0;
}

bool PathHandleGraph::for_each_oriented_step_on_handle_impl(const handle_t& handle,
    const std::function<bool(const oriented_step_handle_t&)>& iteratee) const {
    // Every step on the node becomes an oriented step, oriented to face the
    // same way as the queried handle. So a step whose path runs through the
    // node the way the query handle points is forward along its path, and one
    // whose path runs the other way is backward along its path.
    return for_each_step_on_handle_impl(handle, [&](const step_handle_t& step) -> bool {
        bool is_reverse_along_path = (get_handle_of_step(step) != handle);
        return iteratee(get_oriented_step(step, is_reverse_along_path));
    });
}

oriented_step_handle_t PathHandleGraph::get_oriented_step(const step_handle_t& step_handle, bool is_reverse_along_path) const {
    return step_bool_packing::pack(step_handle, is_reverse_along_path);
}

oriented_step_handle_t PathHandleGraph::flip_along_path(const oriented_step_handle_t& oriented_step_handle) const {
    return step_bool_packing::toggle_bit(oriented_step_handle);
}

handle_t PathHandleGraph::get_handle_of_oriented_step(const oriented_step_handle_t& oriented_step_handle) const {
    // The step's own handle faces the way the path runs through the node. If we
    // are facing backward along the path, we see the node the other way.
    handle_t handle = get_handle_of_step(step_bool_packing::unpack_step(oriented_step_handle));
    return step_bool_packing::unpack_bit(oriented_step_handle) ? flip(handle) : handle;
}

path_handle_t PathHandleGraph::get_path_handle_of_oriented_step(const oriented_step_handle_t& oriented_step_handle) const {
    return get_path_handle_of_step(step_bool_packing::unpack_step(oriented_step_handle));
}

bool PathHandleGraph::get_is_reverse_along_path(const oriented_step_handle_t& oriented_step_handle) const {
    return step_bool_packing::unpack_bit(oriented_step_handle);
}

bool PathHandleGraph::has_next_oriented_step(const oriented_step_handle_t& oriented_step_handle) const {
    // Moving forward in our orientation is moving backward along the path when
    // we face backward, so the path-end we can run into is correspondingly the
    // path's front.
    step_handle_t step = step_bool_packing::unpack_step(oriented_step_handle);
    return step_bool_packing::unpack_bit(oriented_step_handle) ? has_previous_step(step) : has_next_step(step);
}

bool PathHandleGraph::has_previous_oriented_step(const oriented_step_handle_t& oriented_step_handle) const {
    step_handle_t step = step_bool_packing::unpack_step(oriented_step_handle);
    return step_bool_packing::unpack_bit(oriented_step_handle) ? has_next_step(step) : has_previous_step(step);
}

oriented_step_handle_t PathHandleGraph::get_next_step(const oriented_step_handle_t& oriented_step_handle) const {
    // Stepping forward in our orientation keeps the same orientation but walks
    // the path forward when we face forward and backward when we face backward.
    step_handle_t step = step_bool_packing::unpack_step(oriented_step_handle);
    bool is_reverse_along_path = step_bool_packing::unpack_bit(oriented_step_handle);
    step_handle_t next = is_reverse_along_path ? get_previous_step(step) : get_next_step(step);
    return step_bool_packing::pack(next, is_reverse_along_path);
}

oriented_step_handle_t PathHandleGraph::get_previous_oriented_step(const oriented_step_handle_t& oriented_step_handle) const {
    step_handle_t step = step_bool_packing::unpack_step(oriented_step_handle);
    bool is_reverse_along_path = step_bool_packing::unpack_bit(oriented_step_handle);
    step_handle_t prev = is_reverse_along_path ? get_next_step(step) : get_previous_step(step);
    return step_bool_packing::pack(prev, is_reverse_along_path);
}

PathForEachSocket PathHandleGraph::scan_path(const path_handle_t& path) const {
    return PathForEachSocket(this, path);
}
    
PathForEachSocket::PathForEachSocket(const PathHandleGraph* graph, const path_handle_t& path) : graph(graph), path(path) {
    
}
    
PathForEachSocket::iterator PathForEachSocket::begin() const {
    return iterator(graph->path_begin(path), graph->get_is_circular(path) && !graph->is_empty(path), graph);
}
    
PathForEachSocket::iterator PathForEachSocket::end() const {
    // we will end on the beginning step in circular paths
    return iterator(graph->get_is_circular(path) ? graph->path_begin(path) : graph->path_end(path), false, graph);
}
    
PathForEachSocket::iterator::iterator(const step_handle_t& step, bool force_unequal,
                                      const PathHandleGraph* graph) : step(step), force_unequal(force_unequal), graph(graph) {
    
}
    
PathForEachSocket::iterator& PathForEachSocket::iterator::operator++() {
    step = graph->get_next_step(step);
    force_unequal = false;
    return *this;
}

handle_t PathForEachSocket::iterator::operator*() const {
    return graph->get_handle_of_step(step);
}

bool PathForEachSocket::iterator::operator==(const PathForEachSocket::iterator& other) const {
    return !force_unequal && !other.force_unequal && graph == other.graph && step == other.step;
}

bool PathForEachSocket::iterator::operator!=(const PathForEachSocket::iterator& other) const {
    return !(*this == other);
}

}


