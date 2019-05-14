#ifndef HANDLEGRAPH_EMPTY_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_EMPTY_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines a concrete HandleGraph with no nodes in it.
 * Useful for testing purposes, and as an example.
 */
 
#include "handlegraph/handle_graph.hpp"

namespace handlegraph {

/**
 * A minimal concrete HandleGraph implementation, which represents a
 * permanently empty graph with no nodes.
 */
class EmptyHandleGraph : public HandleGraph {

public:
    virtual bool has_node(nid_t node_id) const;
    virtual handle_t get_handle(const nid_t& node_id, bool is_reverse = false) const;
    virtual nid_t get_id(const handle_t& handle) const;
    virtual bool get_is_reverse(const handle_t& handle) const;
    virtual handle_t flip(const handle_t& handle) const;
    virtual size_t get_length(const handle_t& handle) const;
    virtual std::string get_sequence(const handle_t& handle) const;
    virtual size_t node_size() const;
    virtual nid_t min_node_id() const;
    virtual nid_t max_node_id() const;
    
protected:
    virtual bool follow_edges_impl(const handle_t& handle, bool go_left, const std::function<bool(const handle_t&)>& iteratee) const;
    virtual bool for_each_handle_impl(const std::function<bool(const handle_t&)>& iteratee, bool parallel = false) const;

};

}

#endif
