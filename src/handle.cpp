#include "handlegraph/handle_graph.hpp"
#include "handlegraph/path_handle_graph.hpp"
#include "handlegraph/util.hpp"

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
    
bool PathHandleGraph::is_empty(const path_handle_t& path_handle) const {
    // By default, we can answer emptiness queries with the length query.
    // But some implementations may have an expensive length query and a cheaper emptiness one
    return get_occurrence_count(path_handle) == 0;
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

/// Define equality on occurrence handles
bool operator==(const occurrence_handle_t& a, const occurrence_handle_t& b) {
    return as_integers(a)[0] == as_integers(b)[0] && as_integers(a)[1] == as_integers(b)[1];
}

/// Define inequality on occurrence handles
bool operator!=(const occurrence_handle_t& a, const occurrence_handle_t& b) {
    return !(a == b);
}

}


