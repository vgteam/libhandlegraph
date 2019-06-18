#ifndef HANDLEGRAPH_PATH_POSITION_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_PATH_POSITION_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the PathPositionHandleGraph interface for graphs that can have paths altered.
 */

#include "handlegraph/path_handle_graph.hpp"

namespace handlegraph {

/**
 * This is the interface for a path handle graph with path positions
 */
class PathPositionHandleGraph : virtual public PathHandleGraph {
public:

    /// Execute a function on each step (step_handle_t) and its path relative position and orientation
    /// on a handle in any path. If it returns bool and returns false, stop iteration.
    /// Returns true if we finished and false if we stopped early.
    virtual bool for_each_step_position_on_handle(
        const handle_t& handle,
        const std::function<bool(const step_handle_t&, const bool&, const uint64_t&)>& iteratee) const = 0;

};

}

#endif

