#ifndef HANDLEGRAPH_ALGORITHMS_EADES_ALGORITHM_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_EADES_ALGORITHM_HPP_INCLUDED

/**
 * \file eades_algorithm.hpp
 *
 * Implements the Eades, Lin, Smyth (1993) algorithm for computing a heuristic
 * solution to the minimum feedback arc set.
 */

#include "handlegraph/handle_graph.hpp"

#include <vector>

namespace handlegraph {
namespace algorithms {

/// Returns a layout of handles that has a small number of edges that point backward
/// along the layout (i.e. feedback arcs). Only valid for graphs that have a single
/// stranded orientation. Consider checking this property with
/// algorithms::single_stranded_orientation.
std::vector<handle_t> eades_algorithm(const HandleGraph* graph);

}
}

#endif
