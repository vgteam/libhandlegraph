#ifndef HANDLEGRAPH_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the base HandleGraph interface.
 */
 
#include "handlegraph/types.hpp"
#include "handlegraph/iteratee.hpp"

#include <functional>
#include <string>
#include <iostream>

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
    
    /// Write the contents of this graph to an ostream.
    virtual void serialize(std::ostream& out) const = 0;
    
    /// Sets the contents of this graph to the contents of a serialized graph from
    /// an istream. The serialized graph must be from the same implementation of the
    /// HandleGraph interface as is calling deserialize(). Can only be called by an
    /// empty graph.
    virtual void deserialize(std::istream& in) = 0;
   
    /// Method to check if a node exists by ID
    virtual bool has_node(nid_t node_id) const = 0;
   
    /// Look up the handle for the node with the given ID in the given orientation
    virtual handle_t get_handle(const nid_t& node_id, bool is_reverse = false) const = 0;
    
    /// Get the ID from a handle
    virtual nid_t get_id(const handle_t& handle) const = 0;
    
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
    virtual nid_t min_node_id() const = 0;
    
    /// Return the largest ID in the graph, or some larger number if the
    /// largest ID is unavailable. Return value is unspecified if the graph is empty.
    virtual nid_t max_node_id() const = 0;
    
    ////////////////////////////////////////////////////////////////////////////
    // Stock interface that uses backing virtual methods
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
    // Backing protected virtual methods that need to be implemented
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
    
    /// Returns true if there is an edge that allows traversal from the left
    /// handle to the right handle. By default O(n) in the number of edges
    /// on left, but can be overridden with more efficient implementations.
    virtual bool has_edge(const handle_t& left, const handle_t& right) const;
    
    /// Convenient wrapper of has_edge for edge_t argument.
    inline bool has_edge(const edge_t& edge) {
        return has_edge(edge.first, edge.second);
    }
    
    /// Returns one base of a handle's sequence, in the orientation of the
    /// handle.
    virtual char get_base(const handle_t& handle, size_t index) const;
    
    /// Returns a substring of a handle's sequence, in the orientation of the
    /// handle. If the indicated substring would extend beyond the end of the
    /// handle's sequence, the return value is truncated to the sequence's end.
    virtual std::string get_subsequence(const handle_t& handle, size_t index, size_t size) const;
    
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
    
    /// Loop over all edges in their canonical orientation (as returned by
    /// edge_handle) as edge_t items and execute an iteratee on each one. If
    /// the iteratee returns bool, and it returns false, stop iteration. Return
    /// true if the iteration completed and false if it stopped early. If run
    /// in parallel (parallel = true), stopping early is best-effort.
    template<typename Iteratee>
    bool for_each_edge(const Iteratee& iteratee, bool parallel = false) const;
    
};


////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

template<typename Iteratee>
bool HandleGraph::follow_edges(const handle_t& handle, bool go_left, const Iteratee& iteratee) const {
    return follow_edges_impl(handle, go_left, BoolReturningWrapper<Iteratee, handle_t>::wrap(iteratee));
}

template<typename Iteratee>
bool HandleGraph::for_each_handle(const Iteratee& iteratee, bool parallel) const {
    return for_each_handle_impl(BoolReturningWrapper<Iteratee, handle_t>::wrap(iteratee), parallel);
}

template<typename Iteratee>
bool HandleGraph::for_each_edge(const Iteratee& iteratee, bool parallel) const {
    // (If we pre-cast our lambda to std::function we won't generate a new
    // template instantiation for it each time we are instantiated.)
    return for_each_handle((std::function<bool(const handle_t&)>)[&](const handle_t& handle) -> bool {
        bool keep_going = true;
        // Filter to edges where this node is lower ID or any rightward
        // self-loops. 
        follow_edges(handle, false, (std::function<bool(const handle_t&)>)[&](const handle_t& next) -> bool {
            if (get_id(handle) <= get_id(next)) {
                keep_going = iteratee(edge_handle(handle, next));
            }
            return keep_going;
        });
        if (keep_going) {
            // Filter to edges where this node is lower ID or leftward
            // reversing self-loop.
            follow_edges(handle, true, (std::function<bool(const handle_t&)>)[&](const handle_t& prev) -> bool {
                if (get_id(handle) < get_id(prev) ||
                    (get_id(handle) == get_id(prev) && get_is_reverse(prev))) {
                    keep_going = iteratee(edge_handle(prev, handle));
                }
                return keep_going;
            });
        }
        
        return keep_going;
    }, parallel);
}


}

#endif
