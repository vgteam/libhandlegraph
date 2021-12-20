#ifndef HANDLEGRAPH_PATH_SEGMENT_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_PATH_SEGMENT_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the SegmentHandleGraph interface for graphs that have a translation
 * to named segments, as in a GFA that has been chopped.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {

/**
 * This is the interface for a handle graph that stores a mapping back to segments.
 */
class SegmentHandleGraph : virtual public HandleGraph {
public:
    
    virtual ~SegmentHandleGraph() = default;
    
    ////////////////////////////////////////////////////////////////////////////
    // Interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
    /// Returns `true` if the graph contains a translation from node ids to segment names.
    virtual bool has_segment_names() const = 0;

    /// Returns (GFA segment name, semiopen node id range) containing the handle.
    /// If there is no such translation, returns ("id", (id, id + 1)).
    virtual std::pair<std::string, std::pair<nid_t, nid_t>> get_segment(const handle_t& handle) const = 0;

    /// Returns (GFA segment name, starting offset in the same orientation) for the handle.
    /// If there is no translation, returns ("id", 0).
    virtual std::pair<std::string, size_t> get_segment_name_and_offset(const handle_t& handle) const = 0;

    /// Returns the name of the original GFA segment corresponding to the handle.
    /// If there is no translation, returns the node id as a string.
    virtual std::string get_segment_name(const handle_t& handle) const = 0;

    /// Returns the starting offset in the original GFA segment corresponding to the handle
    /// in the same orientation as the handle.
    /// If there is no translation, returns 0.
    virtual size_t get_segment_offset(const handle_t& handle) const = 0;

protected:
    
    /// Calls `iteratee` with each segment name and the semiopen interval of node ids
    /// corresponding to it. Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    virtual bool for_each_segment_impl(const std::function<bool(const std::string&, const std::pair<nid_t, nid_t>&)>& iteratee) const = 0;
    
    /// Calls `iteratee` with each inter-segment edge and the corresponding segment names
    /// in the canonical orientation. Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    virtual bool for_each_link_impl(const std::function<bool(const edge_t&, const std::string&, const std::string&)>& iteratee) const = 0;

public:

    ////////////////////////////////////////////////////////////////////////////
    // Concrete utility methods
    ////////////////////////////////////////////////////////////////////////////

    /// Calls `iteratee` with each segment name as a string, and the semiopen
    /// interval of node ids corresponding to it as a std::pair of nid_t
    /// values. Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    template<typename Iteratee>
    bool for_each_segment(const Iteratee& iteratee) const;

    /// Calls `iteratee` with each inter-segment edge (as an edge_t) and the
    /// corresponding segment names in the canonical orientation (as two
    /// strings). Stops early if the call returns `false`.
    /// Returns false if iteration was stopped, and true otherwise.
    template<typename Iteratee>
    bool for_each_link(const Iteratee& iteratee) const;
};

////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

template<typename Iteratee>
bool SegmentHandleGraph::for_each_segment(const Iteratee& iteratee) const {
    return for_each_segment_impl(BoolReturningWrapper<Iteratee>::wrap(iteratee));
}

template<typename Iteratee>
bool SegmentHandleGraph::for_each_link(const Iteratee& iteratee) const {
    return for_each_link_impl(BoolReturningWrapper<Iteratee>::wrap(iteratee));
}

}

#endif
