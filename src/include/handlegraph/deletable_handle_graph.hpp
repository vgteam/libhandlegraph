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
    /// Destroys any paths in which the node participates.
    /// Invalidates the destroyed handle.
    /// May be called during serial for_each_handle iteration **ONLY** on the node being iterated.
    /// May **NOT** be called during parallel for_each_handle iteration.
    /// May **NOT** be called on the node from which edges are being followed during follow_edges.
    /// May **NOT** be called during iteration over paths, if it would destroy a path.
    /// May **NOT** be called during iteration along a path, if it would destroy that path.
    virtual void destroy_handle(const handle_t& handle) = 0;
    
    /// Remove the edge connecting the given handles in the given order and orientations.
    /// Ignores nonexistent edges.
    /// Does not update any stored paths.
    virtual void destroy_edge(const handle_t& left, const handle_t& right) = 0;
    
    /// Convenient wrapper for destroy_edge.
    inline void destroy_edge(const edge_t& edge) {
        destroy_edge(edge.first, edge.second);
    }
    
    /// Remove all nodes and edges.
    virtual void clear() = 0;
};

}

#endif


