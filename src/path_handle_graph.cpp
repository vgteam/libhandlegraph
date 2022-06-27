#include "handlegraph/path_handle_graph.hpp"

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

PathHandleForEachSocket PathHandleGraph::scan_paths() const {
    PathHandleForEachSocket socket;
    for_each_path_handle([&](const path_handle_t& path_handle) {
        socket.to_iterate.push_back(path_handle);
    });
    return socket;
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


