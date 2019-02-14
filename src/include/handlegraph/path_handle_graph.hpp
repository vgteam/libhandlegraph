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
    
    /// Determine if a path name exists and is legal to get a path handle for.
    virtual bool has_path(const std::string& path_name) const = 0;
    
    /// Look up the path handle for the given path name.
    /// The path with that name must exist.
    virtual path_handle_t get_path_handle(const std::string& path_name) const = 0;
    
    /// Look up the name of a path from a handle to it
    virtual std::string get_path_name(const path_handle_t& path_handle) const = 0;
    
    /// Returns the number of node occurrences in the path
    virtual size_t get_occurrence_count(const path_handle_t& path_handle) const = 0;

    /// Returns the number of paths stored in the graph
    virtual size_t get_path_count() const = 0;
    
    /// Get a node handle (node ID and orientation) from a handle to an occurrence on a path
    virtual handle_t get_occurrence(const occurrence_handle_t& occurrence_handle) const = 0;
    
    /// Get a handle to the first occurrence in a path.
    /// The path MUST be nonempty.
    virtual occurrence_handle_t get_first_occurrence(const path_handle_t& path_handle) const = 0;
    
    /// Get a handle to the last occurrence in a path
    /// The path MUST be nonempty.
    virtual occurrence_handle_t get_last_occurrence(const path_handle_t& path_handle) const = 0;
    
    /// Returns true if the occurrence is not the last occurence on the path, else false
    virtual bool has_next_occurrence(const occurrence_handle_t& occurrence_handle) const = 0;
    
    /// Returns true if the occurrence is not the first occurence on the path, else false
    virtual bool has_previous_occurrence(const occurrence_handle_t& occurrence_handle) const = 0;
    
    /// Returns a handle to the next occurrence on the path
    virtual occurrence_handle_t get_next_occurrence(const occurrence_handle_t& occurrence_handle) const = 0;
    
    /// Returns a handle to the previous occurrence on the path
    virtual occurrence_handle_t get_previous_occurrence(const occurrence_handle_t& occurrence_handle) const = 0;
    
    /// Returns a handle to the path that an occurrence is on
    virtual path_handle_t get_path_handle_of_occurrence(const occurrence_handle_t& occurrence_handle) const = 0;
    
    /// Returns a vector of all occurrences of a node on paths. Optionally restricts to
    /// occurrences that match the handle in orientation.
    virtual std::vector<occurrence_handle_t> occurrences_of_handle(const handle_t& handle,
                                                                   bool match_orientation = false) const = 0;
                                                                   
    ////////////////////////////////////////////////////////////////////////////
    // Stock interface that uses backing virtual methods
    ////////////////////////////////////////////////////////////////////////////
    
    /// Execute a function on each path_handle_t in the graph. If it returns bool, and
    /// it returns false, stop iteration. Returns true if we finished and false if we
    /// stopped early.
    template<typename Iteratee>
    bool for_each_path_handle(const Iteratee& iteratee) const;
    
    /// Execute a function on each occurrence (occurrence_handle_t) of a handle
    /// in any path. If it returns bool and returns false, stop iteration.
    /// Returns true if we finished and false if we stopped early.
    template<typename Iteratee>
    bool for_each_occurrence_on_handle(const handle_t& handle, const Iteratee& iteratee) const;
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing protected virtual methods that need to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
protected:
    
    /// Execute a function on each path in the graph. If it returns false, stop
    /// iteration. Returns true if we finished and false if we stopped early.
    virtual bool for_each_path_handle_impl(const std::function<bool(const path_handle_t&)>& iteratee) const = 0;
    
    /// Execute a function on each occurrence of a handle in any path. If it
    /// returns false, stop iteration. Returns true if we finished and false if
    /// we stopped early.
    virtual bool for_each_occurrence_on_handle_impl(const handle_t& handle,
        const std::function<bool(const occurrence_handle_t&)>& iteratee) const = 0;

public:

    ////////////////////////////////////////////////////////////////////////////
    // Additional optional interface with a default implementation
    ////////////////////////////////////////////////////////////////////////////

    /// Returns true if the given path is empty, and false otherwise
    virtual bool is_empty(const path_handle_t& path_handle) const;

    ////////////////////////////////////////////////////////////////////////////
    // Concrete utility methods
    ////////////////////////////////////////////////////////////////////////////

    /// Loop over all the occurrences (occurrence_handle_t) along a path, from
    /// first through last. If the iteratee returns bool, and it returns false,
    /// stop. Returns true if we finished and false if we stopped early.
    template<typename Iteratee>
    bool for_each_occurrence_in_path(const path_handle_t& path, const Iteratee& iteratee) const;
};

////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

template<typename Iteratee>
bool PathHandleGraph::for_each_path_handle(const Iteratee& iteratee) const {
    return for_each_path_handle_impl(BoolReturningWrapper<Iteratee, path_handle_t>::wrap(iteratee));
}

template<typename Iteratee>
bool PathHandleGraph::for_each_occurrence_on_handle(const handle_t& handle, const Iteratee& iteratee) const {
    return for_each_occurrence_on_handle_impl(handle, BoolReturningWrapper<Iteratee, occurrence_handle_t>::wrap(iteratee));
}


template<typename Iteratee>
bool PathHandleGraph::for_each_occurrence_in_path(const path_handle_t& path, const Iteratee& iteratee) const {
    if (is_empty(path)) {
        // Nothing to do!
        return true;
    }
    
    auto wrapped = BoolReturningWrapper<Iteratee, occurrence_handle_t>::wrap(iteratee);
    
    bool keep_going = true;
    
    // Otherwise the path is nonempty so it is safe to try and grab a first occurrence
    auto here = get_first_occurrence(path);
    // Run for the first occurrence
    keep_going &= wrapped(here);
    while (has_next_occurrence(here) && keep_going) {
        // Run for all subsequent occurrences on the path
        here = get_next_occurrence(here);
        keep_going &= wrapped(here);
    }
    
    return keep_going;
}

}

#endif
