#ifndef HANDLEGRAPH_ALGORITHMS_REVERSE_COMPLEMENT_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_REVERSE_COMPLEMENT_HPP_INCLUDED

/**
 * \file reverse_complement.hpp
 *
 * Defines algorithm for reverse complementing the sequence in a graph
 */

#include "handlegraph/mutable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Fills a MutableHandleGraph 'into' with a graph that has the same sequence and path
/// space as 'source', but the forward strand of every node is flipped to the reverse
/// strand. Reports an error and exits if 'into' is not empty. Node IDs will match between
/// the two graphs.
void reverse_complement_graph(const HandleGraph* source,
                              MutableHandleGraph* into);

}
}

#endif
