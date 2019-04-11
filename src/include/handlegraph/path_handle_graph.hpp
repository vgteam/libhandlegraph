#ifndef HANDLEGRAPH_PATH_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_PATH_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the PathHandleGraph interface for graphs that have embedded paths.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {

/**
 * This is the interface for a handle graph that stores embedded paths.
 */
class PathHandleGraph : virtual public HandleGraph {
public:
    
    ////////////////////////////////////////////////////////////////////////////
    // Path handle interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
    /// Returns the number of paths stored in the graph
    virtual size_t get_path_count() const = 0;
    
    /// Determine if a path name exists and is legal to get a path handle for.
    virtual bool has_path(const std::string& path_name) const = 0;
    
    /// Look up the path handle for the given path name.
    /// The path with that name must exist.
    virtual path_handle_t get_path_handle(const std::string& path_name) const = 0;
    
    /// Look up the name of a path from a handle to it
    virtual std::string get_path_name(const path_handle_t& path_handle) const = 0;
    
    /// Look up whether a path is circular
    virtual bool get_is_circular(const path_handle_t& path_handle) const = 0;
    
    /// Returns the number of node steps in the path
    virtual size_t get_step_count(const path_handle_t& path_handle) const = 0;
    
    /// Get a node handle (node ID and orientation) from a handle to an step on a path
    virtual handle_t get_handle_of_step(const step_handle_t& step_handle) const = 0;
    
    /// Returns a handle to the path that an step is on
    virtual path_handle_t get_path_handle_of_step(const step_handle_t& step_handle) const = 0;
    
    /// Get a handle to the first step, or in a circular path to an arbitrary step
    /// considered "first". If the path is empty, returns the past-the-last step
    /// returned by path_end.
    virtual step_handle_t path_begin(const path_handle_t& path_handle) const = 0;
    
    /// Get a handle to a fictitious position past the end of a path. This position is
    /// return by get_next_step for the final step in a path in a non-circular path.
    /// Note that get_next_step will *NEVER* return this value for a circular path.
    virtual step_handle_t path_end(const path_handle_t& path_handle) const = 0;
    
    /// Returns a handle to the next step on the path. If the given step is the final step
    /// of a non-circular path, returns the past-the-last step that is also returned by
    /// path_end. In a circular path, the "last" step will loop around to the "first" (i.e.
    /// the one returned by path_begin).
    /// Note: to iterate over each step one time, even in a circular path, consider
    /// for_each_step_in_path.
    virtual step_handle_t get_next_step(const step_handle_t& step_handle) const = 0;
    
    /// Returns a handle to the previous step on the path. If the given step is the first
    /// step of a non-circular path, this method has undefined behavior. In a circular path,
    /// it will loop around from the "first" step (i.e. the one returned by path_begin) to
    /// the "last" step.
    /// Note: to iterate over each step one time, even in a circular path, consider
    /// for_each_step_in_path.
    virtual step_handle_t get_previous_step(const step_handle_t& step_handle) const = 0;
    
    ////////////////////////////////////////////////////////////////////////////
    // Stock interface that uses backing virtual methods
    ////////////////////////////////////////////////////////////////////////////
    
    /// Execute a function on each path_handle_t in the graph. If it returns bool, and
    /// it returns false, stop iteration. Returns true if we finished and false if we
    /// stopped early.
    template<typename Iteratee>
    bool for_each_path_handle(const Iteratee& iteratee) const;
    
    /// Execute a function on each step (step_handle_t) of a handle
    /// in any path. If it returns bool and returns false, stop iteration.
    /// Returns true if we finished and false if we stopped early.
    template<typename Iteratee>
    bool for_each_step_on_handle(const handle_t& handle, const Iteratee& iteratee) const;
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing protected virtual methods that need to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
protected:
    
    /// Execute a function on each path in the graph. If it returns false, stop
    /// iteration. Returns true if we finished and false if we stopped early.
    virtual bool for_each_path_handle_impl(const std::function<bool(const path_handle_t&)>& iteratee) const = 0;
    
    /// Execute a function on each step of a handle in any path. If it
    /// returns false, stop iteration. Returns true if we finished and false if
    /// we stopped early.
    virtual bool for_each_step_on_handle_impl(const handle_t& handle,
        const std::function<bool(const step_handle_t&)>& iteratee) const = 0;

public:

    ////////////////////////////////////////////////////////////////////////////
    // Additional optional interface with a default implementation
    ////////////////////////////////////////////////////////////////////////////
    
    /// Returns a vector of all steps of a node on paths. Optionally restricts to
    /// steps that match the handle in orientation.
    virtual std::vector<step_handle_t> steps_of_handle(const handle_t& handle,
                                                       bool match_orientation = false) const;

    /// Returns true if the given path is empty, and false otherwise
    virtual bool is_empty(const path_handle_t& path_handle) const;

    ////////////////////////////////////////////////////////////////////////////
    // Concrete utility methods
    ////////////////////////////////////////////////////////////////////////////

    /// Loop over all the steps (step_handle_t) along a path. In a non-circular
    /// path, iterates from first through last step. In a circular path, iterates
    /// from the step returned by path_begin forward around to the step immediately
    /// before it. If the iteratee returns bool, and it returns false, stop. Returns
    /// true if we finished and false if we stopped early.
    template<typename Iteratee>
    bool for_each_step_in_path(const path_handle_t& path, const Iteratee& iteratee) const;
};

////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

template<typename Iteratee>
bool PathHandleGraph::for_each_path_handle(const Iteratee& iteratee) const {
    return for_each_path_handle_impl(BoolReturningWrapper<Iteratee, path_handle_t>::wrap(iteratee));
}

template<typename Iteratee>
bool PathHandleGraph::for_each_step_on_handle(const handle_t& handle, const Iteratee& iteratee) const {
    return for_each_step_on_handle_impl(handle, BoolReturningWrapper<Iteratee, step_handle_t>::wrap(iteratee));
}


template<typename Iteratee>
bool PathHandleGraph::for_each_step_in_path(const path_handle_t& path, const Iteratee& iteratee) const {

    auto wrapped = BoolReturningWrapper<Iteratee, step_handle_t>::wrap(iteratee);
    // Otherwise the path is nonempty so it is safe to try and grab a first step
    
    // Get the value that the step should be when we are done
    auto end = get_is_circular(path) ? path_begin(path) : path_end(path);
    
    // We might need to ignore the fact that we meet the ending condition in the first
    // iteration on non-empty circular paths
    bool ignore_end = !is_empty(path) && get_is_circular(path);
    // Allow the iteratee to set a bail-out condition
    bool keep_going = true;
    for (auto here = path_begin(path); keep_going && (ignore_end || here != end); here = get_next_step(here)) {
        // Execute the iteratee on this step
        keep_going &= wrapped(here);
        ignore_end = false;
    }
    
    return keep_going;
}

}

#endif
