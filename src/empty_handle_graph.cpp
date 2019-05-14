#include "handlegraph/empty_handle_graph.hpp"

/** \file empty_handle_graph.cpp
 * Implementations for the EmptyHandleGraph class.
 */

namespace handlegraph {

bool EmptyHandleGraph::has_node(nid_t node_id) const {
    return false;
}

handle_t EmptyHandleGraph::get_handle(const nid_t& node_id, bool is_reverse) const {
    throw std::runtime_error("Cannot get any handles from an empty graph");
}

nid_t EmptyHandleGraph::get_id(const handle_t& handle) const {
    return 0;
}

bool EmptyHandleGraph::get_is_reverse(const handle_t& handle) const {
    return false;
}

handle_t EmptyHandleGraph::flip(const handle_t& handle) const {
    throw std::runtime_error("Cannot flip handles to nothing");
}

size_t EmptyHandleGraph::get_length(const handle_t& handle) const {
    return 0;
}

std::string EmptyHandleGraph::get_sequence(const handle_t& handle) const {
    return "";
}

size_t EmptyHandleGraph::node_size() const {
    return 0;
}

nid_t EmptyHandleGraph::min_node_id() const {
    return 0;
}

nid_t EmptyHandleGraph::max_node_id() const {
    return 0;
}

bool EmptyHandleGraph::follow_edges_impl(const handle_t& handle, bool go_left, const std::function<bool(const handle_t&)>& iteratee) const {
    return true;
}

bool EmptyHandleGraph::for_each_handle_impl(const std::function<bool(const handle_t&)>& iteratee, bool parallel) const {
    return true;
}

}
