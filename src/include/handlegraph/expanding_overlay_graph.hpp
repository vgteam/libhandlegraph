#ifndef HANDLEGRAPH_EXPANDING_OVERLAY_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_EXPANDING_OVERLAY_GRAPH_HPP_INCLUDED

/** \file 
 * Defines an interface for overlay graphs that duplicate underlying graph nodes.
 */
 
#include "handle_graph.hpp"

namespace handlegraph {

/**
 * This is the interface for a graph that represents a transformation of some underlying
 * HandleGraph where every node in the overlay corresponds to a node in the underlying
 * graph, but where more than one node in the overlay can map to the same underlying node.
 */
class ExpandingOverlayGraph : public HandleGraph {

public:
    
    virtual ~ExpandingOverlayGraph() = default;
    
    /**
     * Returns the handle in the underlying graph that corresponds to a handle in the
     * overlay
     */
    virtual handle_t get_underlying_handle(const handle_t& handle) const = 0;
};

}

#endif


