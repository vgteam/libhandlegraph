#include "handlegraph/handle_graph.hpp"
#include "handlegraph/mutable_handle_graph.hpp"
#include "handlegraph/path_handle_graph.hpp"
#include "handlegraph/path_position_handle_graph.hpp"
#include "handlegraph/util.hpp"

#include <vector>

/** \file handle.cpp
 * Implement handle graph utility methods, oprtators, and default implementations.
 */

namespace handlegraph {

size_t HandleGraph::get_degree(const handle_t& handle, bool go_left) const {
    size_t count = 0;
    follow_edges(handle, go_left, [&](const handle_t& ignored) {
        // Just manually count every edge we get by looking at the handle in that orientation
        count++;
    });
    return count;
}

handle_t HandleGraph::forward(const handle_t& handle) const {
    return this->get_is_reverse(handle) ? this->flip(handle) : handle;
}

std::pair<handle_t, handle_t> HandleGraph::edge_handle(const handle_t& left, const handle_t& right) const {
    // The degeneracy is between any pair and a pair of the same nodes but reversed in order and orientation.
    // We compare those two pairs and construct the smaller one.
    auto flipped_right = this->flip(right);
    
    if (as_integer(left) > as_integer(flipped_right)) {
        // The other orientation would be smaller.
        return std::make_pair(flipped_right, this->flip(left));
    } else if(as_integer(left) == as_integer(flipped_right)) {
        // Our left and the flipped pair's left would be equal.
        auto flipped_left = this->flip(left);
        if (as_integer(right) > as_integer(flipped_left)) {
            // And our right is too big, so flip.
            return std::make_pair(flipped_right, flipped_left);
        } else {
            // No difference or we're smaller.
            return std::make_pair(left, right);
        }
    } else {
        // We're smaller
        return std::make_pair(left, right);
    }
}

handle_t HandleGraph::traverse_edge_handle(const edge_t& edge, const handle_t& left) const {
    if (left == edge.first) {
        // The cannonical orientation is the one we want
        return edge.second;
    } else if (left == this->flip(edge.second)) {
        // We really want the other orientation
        return this->flip(edge.first);
    } else {
        // This isn't either handle that the edge actually connects. Something has gone wrong.
        throw std::runtime_error("Cannot view edge " +
            std::to_string(this->get_id(edge.first)) + " " + std::to_string(this->get_is_reverse(edge.first)) + " -> " +
            std::to_string(this->get_id(edge.second)) + " " + std::to_string(this->get_is_reverse(edge.second)) +
            " from non-participant " + std::to_string(this->get_id(left)) + " " + std::to_string(this->get_is_reverse(left)));
    }
}

bool HandleGraph::has_edge(const handle_t& left, const handle_t& right) const {
    bool not_seen = true;
    follow_edges(left, false, [&](const handle_t& next) {
        not_seen = (next != right);
        return not_seen;
    });
    return !not_seen;
}

size_t HandleGraph::get_edge_count() const {
    size_t total = 0;
    for_each_edge([&](const edge_t& ignored) {
        total++;
    });
    return total;
};

size_t HandleGraph::get_total_length() const {
    size_t total = 0;
    for_each_handle([&](const handle_t& h) {
        total += get_length(h);
    });
    return total;
};

char HandleGraph::get_base(const handle_t& handle, size_t index) const {
    return get_sequence(handle)[index];
}

std::string HandleGraph::get_subsequence(const handle_t& handle, size_t index, size_t size) const {
    return get_sequence(handle).substr(index, size);
}

void MutableHandleGraph::increment_node_ids(nid_t increment) {
    // Increment IDs by just reassigning IDs and applying the increment as the ID translation
    reassign_node_ids([&](const nid_t& old_id) -> nid_t {
        return old_id + increment;
    });
}

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
    
bool PathHandleGraph::is_empty(const path_handle_t& path_handle) const {
    // By default, we can answer emptiness queries with the length query.
    // But some implementations may have an expensive length query and a cheaper emptiness one
    return get_step_count(path_handle) == 0;
}
    
bool PathPositionHandleGraph::for_each_step_position_on_handle(const handle_t& handle,
                                                                    const std::function<bool(const step_handle_t&, const bool&, const size_t&)>& iteratee) const {
    return for_each_step_on_handle_impl(handle, [&](const step_handle_t& step) {
        return iteratee(step, get_is_reverse(get_handle_of_step(step)) != get_is_reverse(handle), get_position_of_step(step));
    });
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

/// Define equality on handles
bool operator==(const handle_t& a, const handle_t& b) {
    return as_integer(a) == as_integer(b);
}

/// Define inequality on handles
bool operator!=(const handle_t& a, const handle_t& b) {
    return as_integer(a) != as_integer(b);
}

/// Define equality on path handles
bool operator==(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) == as_integer(b);
}

/// Define inequality on path handles
bool operator!=(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) != as_integer(b);
}

/// Define equality on step handles
bool operator==(const step_handle_t& a, const step_handle_t& b) {
    return as_integers(a)[0] == as_integers(b)[0] && as_integers(a)[1] == as_integers(b)[1];
}

/// Define inequality on step handles
bool operator!=(const step_handle_t& a, const step_handle_t& b) {
    return !(a == b);
}

}


