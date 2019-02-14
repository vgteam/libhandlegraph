#ifndef HANDLEGRAPH_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the base HandleGraph interface.
 */
 
#include "handlegraph/types.hpp"
#include "handlegraph/iteratee.hpp"

#include <functional>
#include <string>

namespace handlegraph {


/**
 * This is the interface that a graph that uses handles needs to support.
 * It is also the interface that users should code against.
 */
class HandleGraph {
public:

    ////////////////////////////////////////////////////////////////////////////
    // Interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////
   
    // Method to check if a node exists by ID
    virtual bool has_node(id_t node_id) const = 0;
   
    /// Look up the handle for the node with the given ID in the given orientation
    virtual handle_t get_handle(const id_t& node_id, bool is_reverse = false) const = 0;
    
    /// Get the ID from a handle
    virtual id_t get_id(const handle_t& handle) const = 0;
    
    /// Get the orientation of a handle
    virtual bool get_is_reverse(const handle_t& handle) const = 0;
    
    /// Invert the orientation of a handle (potentially without getting its ID)
    virtual handle_t flip(const handle_t& handle) const = 0;
    
    /// Get the length of a node
    virtual size_t get_length(const handle_t& handle) const = 0;
    
    /// Get the sequence of a node, presented in the handle's local forward
    /// orientation.
    virtual std::string get_sequence(const handle_t& handle) const = 0;
    
    /// Return the number of nodes in the graph
    virtual size_t node_size() const = 0;
    
    /// Return the smallest ID in the graph, or some smaller number if the
    /// smallest ID is unavailable. Return value is unspecified if the graph is empty.
    virtual id_t min_node_id() const = 0;
    
    /// Return the largest ID in the graph, or some larger number if the
    /// largest ID is unavailable. Return value is unspecified if the graph is empty.
    virtual id_t max_node_id() const = 0;
    
    ////////////////////////////////////////////////////////////////////////////
    // Interface that needs backing protected methods implemented
    ////////////////////////////////////////////////////////////////////////////
    
    // We can't actually overload on the bool-vs-void-returning lambdas we want
    // to accomodate even in C++14. See <https://stackoverflow.com/a/6194623>;
    // basically the std::function constructor for void-returning functions can
    // implicitly take other functions and destroy their returned values. So we
    // need to use templates still. We template on the individual unique types
    // of literal lambdas people pass us.
    
    /// Loop over all the handles to next/previous (right/left) nodes. Passes
    /// them to a callback. If called with a bool-returning invocable thing,
    /// can stop early when the function returns false. Returns true if we
    /// finished and false if we stopped early.
    template<typename Iteratee>
    bool follow_edges(const handle_t& handle, bool go_left, const Iteratee& iteratee) const;
    
    /// Loop over all the nodes in the graph in their local forward
    /// orientations, in their internal stored order. If called with a
    /// bool-returning invocable thing, can stop early when the function
    /// returns false. Returns true if we finished and false if we stopped
    /// early. Can be told to run in parallel, in which case stopping after a
    /// false return value is on a best-effort basis and iteration order is not
    /// defined.
    template<typename Iteratee>
    bool for_each_handle(const Iteratee& iteratee, bool parallel = false) const;
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing protected virtual methods for interface
    ////////////////////////////////////////////////////////////////////////////
    
protected:
    
    /// Loop over all the handles to next/previous (right/left) nodes. Passes
    /// them to a callback which returns false to stop iterating and true to
    /// continue. Returns true if we finished and false if we stopped early.
    virtual bool follow_edges_impl(const handle_t& handle, bool go_left, const std::function<bool(const handle_t&)>& iteratee) const = 0;
    
    /// Loop over all the nodes in the graph in their local forward
    /// orientations, in their internal stored order. Stop if the iteratee
    /// returns false. Can be told to run in parallel, in which case stopping
    /// after a false return value is on a best-effort basis and iteration
    /// order is not defined. Returns true if we finished and false if we 
    /// stopped early.
    virtual bool for_each_handle_impl(const std::function<bool(const handle_t&)>& iteratee, bool parallel = false) const = 0;
    

    
    ////////////////////////////////////////////////////////////////////////////
    // Additional optional interface with a default implementation
    ////////////////////////////////////////////////////////////////////////////
    
public:
    
    /// Get the number of edges on the right (go_left = false) or left (go_left
    /// = true) side of the given handle. The default implementation is O(n) in
    /// the number of edges returned, but graph implementations that track this
    /// information more efficiently can override this method.
    virtual size_t get_degree(const handle_t& handle, bool go_left) const;
    
    ////////////////////////////////////////////////////////////////////////////
    // Concrete utility methods
    ////////////////////////////////////////////////////////////////////////////
    
    /// Get the locally forward version of a handle
    handle_t forward(const handle_t& handle) const;
    
    /// A pair of handles can be used as an edge. When so used, the handles have a
    /// canonical order and orientation.
    edge_t edge_handle(const handle_t& left, const handle_t& right) const;
    
    /// Such a pair can be viewed from either inward end handle and produce the
    /// outward handle you would arrive at.
    handle_t traverse_edge_handle(const edge_t& edge, const handle_t& left) const;
    
    /// Loop over all edges in their canonical orientation (as returned by edge_handle) and
    /// execute an iteratee on each one. Can stop early by returning false from the iteratee.
    void for_each_edge(const std::function<bool(const edge_t&)>& iteratee, bool parallel = false) const;
    
};


// Template implementations

// follow_edges

template<typename Iteratee>
bool HandleGraph::follow_edges(const handle_t& handle, bool go_left, const Iteratee& iteratee) const {
    return follow_edges_impl(handle, go_left, BoolReturningWrapper<Iteratee, handle_t>::wrap(iteratee));
}

// for_each_handle

template<typename Iteratee>
bool HandleGraph::for_each_handle(const Iteratee& iteratee, bool parallel) const {
    return for_each_handle_impl(BoolReturningWrapper<Iteratee, handle_t>::wrap(iteratee), parallel);
}

}

#endif
