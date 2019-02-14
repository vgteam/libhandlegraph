#ifndef HANDLEGRAPH_INTERNAL_MUTABLE_PATH_DELETABLE_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_INTERNAL_MUTABLE_PATH_DELETABLE_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the MutablePathDeletableeHandleGraph interface for graphs that can have paths changed and graph material deleted.
 */

#include "handlegraph/internal/mutable_path_handle_graph.hpp"
#include "handlegraph/internal/deletable_handle_graph.hpp"

namespace handlegraph {


/**
 * This is the interface for a graph which is deletable and which has paths which are also mutable.
 */
class MutablePathDeletableHandleGraph : virtual public MutablePathHandleGraph, virtual public DeletableHandleGraph {
    
    // No extra methods. Deleting a node or edge that is contained in a path is undefined behavior.
};

}

#endif

