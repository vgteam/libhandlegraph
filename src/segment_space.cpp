#include "handlegraph/segment_space.hpp"

/** \file segment_space.cpp
 * Provide SegmentSpace interface default implementations
 */
 
namespace handlegraph {

std::pair<std::string, std::pair<nid_t, nid_t>> SegmentSpace::get_segment(const handle_t& handle, const HandleGraph* graph) const {
    // If the graph is passed, we might not implement HandleGraph. If it isn't, we must implement HandleGraph.
    const HandleGraph* effective_graph = graph ? graph : dynamic_cast<const HandleGraph*>(this);
    if (!effective_graph) {
        throw std::logic_error("Attempted to use a handle with a SegmentSet without a HandleGraph");
    }
    // Do the actual lookup by ID.
    return get_segment(effective_graph->get_id(handle));
}

std::pair<std::string, size_t> SegmentSpace::get_segment_name_and_offset(const handle_t& handle, const HandleGraph* graph) const {
    // If the graph is passed, we might not implement HandleGraph. If it isn't, we must implement HandleGraph.
    const HandleGraph* effective_graph = graph ? graph : dynamic_cast<const HandleGraph*>(this);
    if (!effective_graph) {
        throw std::logic_error("Attempted to use a handle with a SegmentSet without a HandleGraph");
    }
    // Do the actual lookup by ID.
    return get_segment_name_and_offset(effective_graph->get_id(handle), effective_graph->get_is_reverse(handle));
}

std::string SegmentSpace::get_segment_name(const handle_t& handle, const HandleGraph* graph) const {
    // If the graph is passed, we might not implement HandleGraph. If it isn't, we must implement HandleGraph.
    const HandleGraph* effective_graph = graph ? graph : dynamic_cast<const HandleGraph*>(this);
    if (!effective_graph) {
        throw std::logic_error("Attempted to use a handle with a SegmentSet without a HandleGraph");
    }
    // Do the actual lookup by ID.
    return get_segment_name(effective_graph->get_id(handle));
}

size_t SegmentSpace::get_segment_offset(const handle_t& handle, const HandleGraph* graph) const {
    // If the graph is passed, we might not implement HandleGraph. If it isn't, we must implement HandleGraph.
    const HandleGraph* effective_graph = graph ? graph : dynamic_cast<const HandleGraph*>(this);
    if (!effective_graph) {
        throw std::logic_error("Attempted to use a handle with a SegmentSet without a HandleGraph");
    }
    // Do the actual lookup by ID.
    return get_segment_offset(effective_graph->get_id(handle), effective_graph->get_is_reverse(handle));
}

}


