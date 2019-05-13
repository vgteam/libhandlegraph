#include "handlegraph/handle_graph.hpp"
#include "handlegraph/path_handle_graph.hpp"
#include "handlegraph/util.hpp"
#include "handlegraph/mpl.hpp"

// Make sure to include all the HPPs so we syntax-check them at library compile time.
#include "handlegraph/mutable_handle_graph.hpp"
#include "handlegraph/mutable_path_handle_graph.hpp"
#include "handlegraph/deletable_handle_graph.hpp"
#include "handlegraph/mutable_path_mutable_handle_graph.hpp"
#include "handlegraph/mutable_path_deletable_handle_graph.hpp"

#include <vector>

/** \file handle.cpp
 * Implement handle graph utility methods, oprtators, and default implementations.
 */

namespace handlegraph {

// Test the fancy trait system at compile time

// Make sure highest_set_bit works
static_assert(highest_set_bit<0>() == 0);
static_assert(highest_set_bit<1>() == 1);
static_assert(highest_set_bit<2>() == 2);
static_assert(highest_set_bit<4>() == 4);
static_assert(highest_set_bit<10>() == 8);

// We need to make sure we have the right subtype relationships in the full interface.
static_assert(std::is_base_of<HandleGraph, HandleGraphWith<>>::value);
static_assert(std::is_base_of<HandleGraph, HandleGraphWith<PathSupport>>::value);
static_assert(std::is_base_of<PathSupport, HandleGraphWith<PathSupport>>::value);
static_assert(std::is_base_of<HandleGraph, HandleGraphWith<MutablePaths>>::value);
static_assert(std::is_base_of<HandleGraphWith<Deletable, Mutable>, HandleGraphWith<MutablePaths, Mutable, Deletable>>::value);

// And that HandleGraph and HandleGraphWith<> are the same type, so either can match the other.
static_assert(std::is_same<HandleGraph, HandleGraphWith<>>::value);

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

std::vector<step_handle_t> PathSupport::steps_of_handle(const handle_t& handle,
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
    
bool PathSupport::is_empty(const path_handle_t& path_handle) const {
    // By default, we can answer emptiness queries with the length query.
    // But some implementations may have an expensive length query and a cheaper emptiness one
    return get_step_count(path_handle) == 0;
}
    
PathForEachSocket PathSupport::scan_path(const path_handle_t& path) const {
    return PathForEachSocket(this, path);
}
    
PathForEachSocket::PathForEachSocket(const PathSupport* graph, const path_handle_t& path) : graph(graph), path(path) {
    
}
    
PathForEachSocket::iterator PathForEachSocket::begin() const {
    return iterator(graph->path_begin(path), graph->get_is_circular(path) && !graph->is_empty(path), graph);
}
    
PathForEachSocket::iterator PathForEachSocket::end() const {
    // we will end on the beginning step in circular paths
    return iterator(graph->get_is_circular(path) ? graph->path_begin(path) : graph->path_end(path), false, graph);
}
    
PathForEachSocket::iterator::iterator(const step_handle_t& step, bool force_unequal,
                                      const PathSupport* graph) : step(step), force_unequal(force_unequal), graph(graph) {
    
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


