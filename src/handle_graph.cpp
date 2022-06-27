#include "handlegraph/handle_graph.hpp"

#include "handlegraph/util.hpp"

/** \file handle_graphs.cpp
 * Implement handle graph methods.
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

HandleForEachSocket HandleGraph::scan_handles() const {
    HandleForEachSocket socket;
    for_each_handle([&](const handle_t& handle) {
        socket.to_iterate.push_back(handle);
    });
    return socket;
}

HandleForEachSocket HandleGraph::scan_neighbors(const handle_t& handle, bool go_left) const {
    HandleForEachSocket socket;
    follow_edges(handle, go_left, [&](const handle_t& neighbor) {
        socket.to_iterate.push_back(neighbor);
    });
    return socket;
}

EdgeForEachSocket HandleGraph::scan_edges() const {
    EdgeForEachSocket socket;
    for_each_edge([&](const edge_t& edge) {
        socket.to_iterate.push_back(edge);
    });
    return socket;
}

EdgeForEachSocket HandleGraph::scan_edges_of_handle(const handle_t& handle, bool to_left, bool to_right) const {
    EdgeForEachSocket socket;
    if (to_left) {
        follow_edges(handle, true, [&](const handle_t& neighbor) {
            socket.to_iterate.push_back(edge_handle(neighbor, handle));
        });
    }
    if (to_right) {
        follow_edges(handle, false, [&](const handle_t& neighbor) {
            if (neighbor != handle || !to_left) {
                socket.to_iterate.push_back(edge_handle(handle, neighbor));
            }
        });
    }
    
    return socket;
}


}


