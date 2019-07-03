#ifndef HANDLEGRAPH_PATH_POSITION_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_PATH_POSITION_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the PathPositionHandleGraph interface.
 */

#include "handlegraph/path_handle_graph.hpp"
#include "handlegraph/iteratee.hpp"

namespace handlegraph {

/**
 * This is the interface for a path handle graph with path positions
 */
class PathPositionHandleGraph : virtual public PathHandleGraph {
public:
    
    ////////////////////////////////////////////////////////////////////////////
    // Interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////

    /// Returns the length of a path measured in bases of sequence.
    virtual size_t get_path_length(const path_handle_t& path_handle) const = 0;
    
    /// Returns the position along the path of the beginning of this step measured in
    /// bases of sequence. In a circular path, positions start at the step returned by
    /// path_begin().
    virtual size_t get_position_of_step(const step_handle_t& step) const = 0;
    
    /// Returns the step at this position, measured in bases of sequence starting at
    /// the step returned by path_begin(). If the position is past the end of the
    /// path, returns path_end().
    virtual step_handle_t get_step_at_position(const path_handle_t& path,
                                               const size_t& position) const = 0;
    
    ////////////////////////////////////////////////////////////////////////////
    // Additional optional interface with a default implementation
    ////////////////////////////////////////////////////////////////////////////
    
    /// Execute an iteratee on each step on a path, along with its orientation relative to
    /// the path (true if it is reverse the orientation of the handle on the path), and its
    /// position measured in bases of sequence along the path. Positions are always measured
    /// on the forward strand.
    ///
    /// Iteration will stop early if the iteratee returns false. This method returns false if
    /// iteration was stopped early, else true
    virtual bool for_each_step_position_on_handle(const handle_t& handle,
                                                  const std::function<bool(const step_handle_t&, const bool&, const size_t&)>& iteratee) const;
    
    
};
    

}

#endif

