#ifndef HANDLEGRAPH_ALGORITHMS_SPLIT_STRANDS_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_SPLIT_STRANDS_HPP_INCLUDED

/**
 * \file split_strands.hpp
 *
 * Defines algorithm for converting any graph into a single stranded graph.
 */

#include <unordered_map>

#include "handlegraph/mutable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Fills a MutableHandleGraph 'into' with a graph that has the same sequence and path
/// space as 'source', but all of the sequences are on the forward strand. This is
/// accomplished by creating a new node for each node in the source graph with the reverse
/// complement sequence. Returns a map that translates forward-oriented handles from 'into' to
/// the corresponding handle in 'source'. Reports an error and exits if 'into' is not
/// empty.
std::unordered_map<handle_t, handle_t> split_strands(const HandleGraph* source,
                                                     MutableHandleGraph* into);

}
}

#endif
