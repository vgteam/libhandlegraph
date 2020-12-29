#ifndef HANDLEGRAPH_ALGORITHMS_TOPOLOGICAL_SORT_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_TOPOLOGICAL_SORT_HPP_INCLUDED

/**
 * \file topological_sort.hpp
 *
 * Defines a topological sort algorithm for handle graphs.
 */
#include <vector>

#include "handlegraph/handle_graph.hpp"
#include "handlegraph/algorithms/is_single_stranded.hpp"

namespace handlegraph {
namespace algorithms {

/**
 * Order and the nodes in the graph using a topological sort. The sort is
 * guaranteed to be machine-independent (at the cost of O(n log n) run time).
 * The algorithm is well-defined on non-DAG graphs, but the order is
 * not a topological order on these graphs.
 */
std::vector<handle_t> topological_order(const HandleGraph* g);

/**
 * Order the nodes in a graph using a topological sort. The sort is NOT guaranteed
 * to be machine-independent, but it is faster than topological_order(). This algorithm 
 * is invalid in a graph that has any cycles. For safety, consider this property with
 * algorithms::is_directed_acyclic().
 */
std::vector<handle_t> lazy_topological_order(const HandleGraph* g);
    
/**
 * Order the nodes in a graph using a topological sort. Similar to lazy_topological_order
 * but somewhat faster. The algorithm is invalid in a graph that has any cycles or
 * any reversing edges. For safety, consider these properties with algorithms::is_acyclic()
 * and algorithms::is_single_stranded().
 */
std::vector<handle_t> lazier_topological_order(const HandleGraph* g);
                                                      
}
}

#endif
