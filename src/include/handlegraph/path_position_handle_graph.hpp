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

    /// Returns the length of a path measured in bases of sequence.
    virtual size_t path_length(const path_handle_t& path_handle) const = 0;
    
    /// Returns the position along the path of the beginning of this step measured in
    /// bases of sequence. In a circular path, positions start at the step returned by
    /// path_begin().
    virtual size_t get_position_of_step(const step_handle_t& step) const = 0;
    
    /// Returns the step at this position, measured in bases of sequence starting at
    /// the step returned by path_begin(). If the position is past the end of the
    /// path, returns path_end().
    virtual step_handle_t get_step_at_position(const size_t& position) const = 0;
    
    /// Execute an iteratee on each step on a path, along with its orientation relative to
    /// the path (true if it is reverse the orientation of the handle on the path), and its
    /// position measured in bases of sequence along the path. Positions are always measured
    /// on the forward strand. The iteratee should accept arguments in the form:
    /// (const step_handle_t&, const bool&, const size_t&).
    /// The iteratee may return void or a bool, in which case iteration will stop early
    /// if the iteratee returns false. This method returns false if iteration was stopped
    /// early, else true
    template<typename Iteratee>
    bool for_each_step_position_on_handle(const handle_t& handle, const Iteratee& iteratee) const;
    
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing protected virtual method, which can be overridden
    ////////////////////////////////////////////////////////////////////////////
    
protected:

    /// Execute an itteratee on each step and its path relative position and orientation
    /// on a handle in any path. Iteration will stop early if the iteratee returns false.
    /// This method returns false if iteration was stopped early, else true.
    virtual bool for_each_step_position_on_handle_impl(const handle_t& handle,
                                                       const std::function<bool(const step_handle_t&, const bool&, const size_t&)>& iteratee) const;
};
    
    ////////////////////////////////////////////////////////////////////////////
    // Template Implementation
    ////////////////////////////////////////////////////////////////////////////
    
    template<typename Iteratee>
    bool PathPositionHandleGraph::for_each_step_position_on_handle(const handle_t& handle,
                                                                   const Iteratee& iteratee) const {
        return for_each_step_position_on_handle_impl(BoolReturningWrapper<Iteratee, handle_t>::wrap(iteratee));
    }
    
    
    

}

#endif

