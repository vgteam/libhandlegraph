#ifndef HANDLEGRAPH_ALGORITHMS_FIND_TIPS_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_FIND_TIPS_HPP_INCLUDED

/**
 * \file find_tips.hpp
 *
 * Defines algorithms for finding heads, tails, and general tips.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {
namespace algorithms {


/// Find all of the nodes with no edges on their left sides.
std::vector<handle_t> head_nodes(const HandleGraph* g);

/// Find all of the nodes with no edges on their right sides.
std::vector<handle_t> tail_nodes(const HandleGraph* g);

/// Find all of the tips in the graph, facing inward.
/// Nodes with no edges will appear once in each orientation.
std::vector<handle_t> find_tips(const HandleGraph* g);

}
}

#endif
