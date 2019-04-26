#ifndef HANDLEGRAPH_MUTABLE_PATH_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_MUTABLE_PATH_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the MutablePathHandleGraph interface for graphs that can have paths altered.
 */

#include "handlegraph/path_handle_graph.hpp"

namespace handlegraph {

/**
 * This is the interface for a handle graph with embedded paths where the paths can be modified.
 * Note that if the *graph* can also be modified, the implementation will also
 * need to inherit from MutableHandleGraph, via the combination
 * MutablePathMutableHandleGraph interface.
 * TODO: This is a very limited interface at the moment. It will probably need to be extended.
 */
class MutablePathHandleGraph : virtual public PathHandleGraph {
public:
    
    /**
     * Destroy the given path. Invalidates handles to the path and its steps.
     */
    virtual void destroy_path(const path_handle_t& path) = 0;

    /**
     * Create a path with the given name. The caller must ensure that no path
     * with the given name exists already, or the behavior is undefined.
     * Returns a handle to the created empty path. Handles to other paths must
     * remain valid.
     */
    virtual path_handle_t create_path_handle(const std::string& name,
                                             bool is_circular = false) = 0;
    
    /**
     * Append a visit to a node to the given path. Returns a handle to the new
     * final step on the path which is appended. If the path is cirular, the new
     * step is placed between the steps considered "last" and "first" by the
     * method path_begin. Handles to prior steps on the path, and to other paths,
     * must remain valid.
     */
    virtual step_handle_t append_step(const path_handle_t& path, const handle_t& to_append) = 0;
    
    /**
     * Prepend a visit to a node to the given path. Returns a handle to the new
     * first step on the path which is appended. If the path is cirular, the new
     * step is placed between the steps considered "last" and "first" by the
     * method path_begin. Handles to later steps on the path, and to other paths,
     * must remain valid.
     */
    virtual step_handle_t prepend_step(const path_handle_t& path, const handle_t& to_prepend) = 0;
    
    /**
     * Delete a segment of a path and rewrite it as some other sequence of steps. Returns a pair
     * of step_handle_t's that indicate the range of the new segment in the path. The segment to
     * delete should be designated by the first and the past-the-last step handle.  If the step
     * that is returned by path_begin is deleted, path_begin will now return the first step from
     * the new segment or, in the case that the new segment is empty, segment_end.
     */
    virtual std::pair<step_handle_t, step_handle_t> rewrite_segment(const step_handle_t& segment_begin,
                                                                    const step_handle_t& segment_end,
                                                                    const vector<handle_t>& new_segment) = 0;
    
    /**
     * Make a path circular or non-circular. If the path is becoming circular, the
     * last step is joined to the first step. If the path is becoming linear, the
     * step considered "last" is unjoined from the step considered "first" according
     * to the method path_begin.
     */
    virtual void set_circularity(const path_handle_t& path, bool circular) = 0;
};


}

#endif

