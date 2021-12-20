#ifndef HANDLEGRAPH_PATH_SEGMENT_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_PATH_SEGMENT_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the SegmentSpace interface for translations from handle/node ID
 * graphs to named segments, as in a GFA that has been chopped.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {

/**
 * This is the interface for a handle graph that stores a mapping back to segments.
 */
class SegmentSpace {
public:
    
    virtual ~SegmentSpace() = default;
    
    ////////////////////////////////////////////////////////////////////////////
    // Interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
    /// Returns `true` if the object contains a translation from node ids to segment names.
    /// TODO: Users must call this and not make any other queries if it returns false.
    virtual bool has_segment_names() const = 0;
    
    /// Returns (GFA segment name, semiopen node id range) containing the given node.
    /// If there is no such translation, returns ("id", (id, id + 1)).
    virtual std::pair<std::string, std::pair<nid_t, nid_t>> get_segment(const nid_t& node_id) const = 0;

    /// Returns (GFA segment name, starting offset in the same orientation) for
    /// the given node in the given orientation.
    /// If there is no translation, returns ("id", 0).
    ///
    /// TODO: The node's local forward orientation is required by the data
    /// model to match that of the segment. 
    virtual std::pair<std::string, size_t> get_segment_name_and_offset(const nid_t& node_id, bool is_reverse) const = 0;

    /// Returns the name of the original GFA segment corresponding to the handle.
    /// If there is no translation, returns the node id as a string.
    virtual std::string get_segment_name(const nid_t& node_id) const = 0;

    /// Returns the starting offset in the original GFA segment corresponding to the handle
    /// in the same orientation as the handle.
    /// If there is no translation, returns 0.
    ///
    /// TODO: The node's local forward orientation is required by the data
    /// model to match that of the segment. 
    virtual size_t get_segment_offset(const nid_t& node_id, bool is_reverse) const = 0;

protected:
    
    /// Calls `iteratee` with each segment name and the semiopen interval of node ids
    /// corresponding to it. Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    virtual bool for_each_segment_impl(const std::function<bool(const std::string&, const std::pair<nid_t, nid_t>&)>& iteratee) const = 0;
    
    /// Calls `iteratee` with each inter-segment edge and the corresponding segment names
    /// in the canonical orientation. Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    ///
    /// If the current object is not a HandleGraph, graph must be set to a
    /// HandleGraph to use to create the edge_t handles. 
    virtual bool for_each_link_impl(const std::function<bool(const edge_t&, const std::string&, const std::string&)>& iteratee, const HandleGraph* graph = nullptr) const = 0;

public:

    ////////////////////////////////////////////////////////////////////////////
    // Interface with a default implementation
    ////////////////////////////////////////////////////////////////////////////
    
    // These handle-based methods delegate by default to the ID, orientation implementations above.

    /// Returns (GFA segment name, semiopen node id range) containing the handle.
    /// If there is no such translation, returns ("id", (id, id + 1)).
    ///
    /// If the current object is not also a HandleGraph, graph must be set to
    /// the HandleGraph to which the handle belongs.
    virtual std::pair<std::string, std::pair<nid_t, nid_t>> get_segment(const handle_t& handle, const HandleGraph* graph = nullptr) const;

    /// Returns (GFA segment name, starting offset in the same orientation) for the handle.
    /// If there is no translation, returns ("id", 0).
    ///
    /// If the current object is not also a HandleGraph, graph must be set to
    /// the HandleGraph to which the handle belongs.
    virtual std::pair<std::string, size_t> get_segment_name_and_offset(const handle_t& handle, const HandleGraph* graph = nullptr) const;

    /// Returns the name of the original GFA segment corresponding to the handle.
    /// If there is no translation, returns the node id as a string.
    ///
    /// If the current object is not also a HandleGraph, graph must be set to
    /// the HandleGraph to which the handle belongs.
    virtual std::string get_segment_name(const handle_t& handle, const HandleGraph* graph = nullptr) const ;

    /// Returns the starting offset in the original GFA segment corresponding to the handle
    /// in the same orientation as the handle.
    /// If there is no translation, returns 0.
    ///
    /// If the current object is not also a HandleGraph, graph must be set to
    /// the HandleGraph to which the handle belongs.
    virtual size_t get_segment_offset(const handle_t& handle, const HandleGraph* graph = nullptr) const;

    ////////////////////////////////////////////////////////////////////////////
    // Concrete utility methods
    ////////////////////////////////////////////////////////////////////////////

    /// Calls `iteratee` with each segment name as a string, and the semiopen
    /// interval of node ids corresponding to it as a std::pair of nid_t
    /// values. Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    ///
    /// TODO: multiple ranges or nodes with orientations opposing the segment
    /// are not supported by the data model.
    template<typename Iteratee>
    bool for_each_segment(const Iteratee& iteratee) const;

    /// Calls `iteratee` with each inter-segment edge (as an edge_t) and the
    /// corresponding segment names in the canonical orientation (as two
    /// strings). Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    ///
    /// If the current object is not a HandleGraph, graph must be set to a
    /// HandleGraph to use to create the edge_t handles. 
    template<typename Iteratee>
    bool for_each_link(const Iteratee& iteratee, const HandleGraph* graph = nullptr) const;
};

////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

template<typename Iteratee>
bool SegmentSpace::for_each_segment(const Iteratee& iteratee) const {
    return for_each_segment_impl(BoolReturningWrapper<Iteratee>::wrap(iteratee));
}

template<typename Iteratee>
bool SegmentSpace::for_each_link(const Iteratee& iteratee, const HandleGraph* graph) const {
    return for_each_link_impl(BoolReturningWrapper<Iteratee>::wrap(iteratee), graph);
}

}

#endif
