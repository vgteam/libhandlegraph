#ifndef HANDLEGRAPH_DELETABLE_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_DELETABLE_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the DeletableHandleGraph interface for graphs that can have material removed.
 */

#include "handlegraph/mutable_handle_graph.hpp"

namespace handlegraph {

/**
 * This is the interface for a handle graph that supports both addition of new nodes and edges
 * as well as deletion of nodes and edges.
 */
class DeletableHandleGraph : virtual public MutableHandleGraph {
public:
    
    virtual ~DeletableHandleGraph() = default;
    
    /// Remove the node belonging to the given handle and all of its edges.
    /// Either destroys any paths in which the node participates, or leaves a
    /// "hidden", un-iterateable handle in the path to represent the sequence
    /// of the removed node.
    /// Invalidates the destroyed handle.
    /// May be called during serial for_each_handle iteration **ONLY** on the node being iterated.
    /// May **NOT** be called during parallel for_each_handle iteration.
    /// May **NOT** be called on the node from which edges are being followed during follow_edges.
    /// May **NOT** be called during iteration over paths, if it could destroy a path.
    /// May **NOT** be called during iteration along a path, if it could destroy that path.
    virtual void destroy_handle(const handle_t& handle) = 0;
    
    /// Change the sequence of handle's forward orientation to a new sequence. Returns a (possibly altered)
    /// handle to the node with the new sequence. The returned handle has the same orientation as the
    /// handle provided as an argument. May invalidate existing handles to the node. Updates paths to
    /// follow the new node if called through a class inheriting a MutablePathHandleGraph interface.
    virtual handle_t change_sequence(const handle_t& handle, const std::string& sequence);
    
    /// Remove the edge connecting the given handles in the given order and orientations.
    /// Ignores nonexistent edges.
    /// Does not update any stored paths.
    virtual void destroy_edge(const handle_t& left, const handle_t& right) = 0;
    
    /// Shorten a node by truncating either the left or right side of the node, relative to the orientation
    /// of the handle, starting from a given offset along the nodes sequence. Any edges on the truncated
    /// end of the node are deleted. Returns a (possibly altered) handle to the truncated node.
    /// May invalid stored paths.
    virtual handle_t truncate_handle(const handle_t& handle, bool trunc_left, size_t offset);
    
    /// Convenient wrapper for destroy_edge.
    inline void destroy_edge(const edge_t& edge) {
        destroy_edge(edge.first, edge.second);
    }
    
    /// Remove all nodes and edges. May also remove all paths, if applicable.
    virtual void clear() = 0;
};

}

#endif


