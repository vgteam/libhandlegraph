#ifndef HANDLEGRAPH_PATH_FOR_EACH_SOCKET_HPP_INCLUDED
#define HANDLEGRAPH_PATH_FOR_EACH_SOCKET_HPP_INCLUDED

/** \file 
 * Defines the PathForEachSocket adapter, for iterating over paths.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {

// Forward-declare the PathHandleGraph. Its file includes us.
class PathHandleGraph;

/**
 * An auxilliary class that enables for each loops over paths. Not intended to
 * constructed directly. Instead, use the PathHandleGraph's scan_path method.
 */
class PathForEachSocket {
public:
    
    class iterator;
    
    // Get iterator to the first step in the path
    iterator begin() const;

    // Get the end iterator, which we would obtain from get_next_step(path_end(path))
    iterator end() const;
    
    /**
     * Iterator object over path
     */
    class iterator {
    public:
        
        // define all the methods of the unidirectional iterator interface
        
        iterator(const iterator& other) = default;
        iterator& operator=(const iterator& other) = default;
        iterator& operator++();
        handle_t operator*() const;
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
        
        ~iterator() = default;
    private:
        
        // don't allow an iterator to point to nothing
        iterator() = delete;
        // we make this private so that it's only visible from inside
        // the friend class, PathForEachSocket
        iterator(const step_handle_t& step, bool force_unequal,
                 const PathHandleGraph* graph);
        
        /// the step that this iterator points to
        step_handle_t step;
        
        /// a bit of an ugly hack we need to handle the fact that the first
        /// iteration of a circular path is also the place where we will end
        bool force_unequal;
        
        /// the graph that contains the path
        const PathHandleGraph* graph;
        
        friend class PathForEachSocket;
    };
    
    ~PathForEachSocket() = default;
    
private:
    // don't allow a for each socket to no graph or path
    PathForEachSocket() = delete;
    // we make this private so that it's only visible from inside the
    // friend class, PathHandleGraph
    PathForEachSocket(const PathHandleGraph* graph, const path_handle_t& path);
    
    /// The graph that contains the path
    const PathHandleGraph* graph;
    /// The path we're iterating over
    const path_handle_t path;
    
    friend class PathHandleGraph;
};

}

#endif
