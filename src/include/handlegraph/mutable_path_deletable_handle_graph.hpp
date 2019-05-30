#ifndef HANDLEGRAPH_MUTABLE_PATH_DELETABLE_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_MUTABLE_PATH_DELETABLE_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the MutablePathDeletableeHandleGraph interface for graphs that can have paths changed and graph material deleted.
 */

#include "handlegraph/mutable_path_mutable_handle_graph.hpp"
#include "handlegraph/deletable_handle_graph.hpp"

namespace handlegraph {


/**
 * This is the interface for a graph which is deletable and which has paths which are also mutable.
 */
class MutablePathDeletableHandleGraph : virtual public MutablePathMutableHandleGraph, virtual public DeletableHandleGraph {
    
    // Few extra methods. Deleting a node or edge that is contained in a path is undefined behavior.
    // The method clear() is now assumed to delete paths as well as nodes and edges.

public:

    /// Copy over another graph.  Only valid if *this is an empty graph
    virtual void copy(const PathHandleGraph* other);
};

}

#endif

