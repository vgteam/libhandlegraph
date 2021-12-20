#ifndef HANDLEGRAPH_NAMED_NODE_BACK_TRANSLATION_HPP_INCLUDED
#define HANDLEGRAPH_NAMED_NODE_BACK_TRANSLATION_HPP_INCLUDED

/** \file 
 * Defines the NamedNodeBackTranslation interface for translations from
 * oriented base ranges on nodes in a graph, "back" to oriented base ranges on
 * nodes in some other graph, where the destination graph nodes have names.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {

/**
 * Represents a range along an oriented node in a graph. The range is counted
 * on the specified strand of the node, and consists of a start and a length.
 */
using oriented_node_range_t = std::tuple<nid_t, bool, size_t, size_t>;

/**
 * This is the interface for a handle graph that stores a mapping back to other named nodes.
 */
class NamedNodeBackTranslation {
public:
    
    virtual ~NamedNodeBackTranslation() = default;
    
    ////////////////////////////////////////////////////////////////////////////
    // Interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
    /**
     * Translate the given range of bases on the given orientation of the given
     * node in the current graph, to zero or more ranges on orientations of
     * nodes in some prior graph.
     */
    virtual std::vector<oriented_node_range_t> translate_back(const oriented_node_range_t& range) = 0;
    
    /**
     * Get the name of a node in the graph that translate_back() translates
     * into, given its number.
     */
    virtual std::string get_back_graph_node_name(const nid_t& back_node_id) = 0;


};

}

#endif
