#ifndef HANDLEGRAPH_ALGORITHMS_DAGIFY_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_DAGIFY_HPP_INCLUDED


#include <unordered_map>

#include "handlegraph/mutable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {


// Fill an empty MutableHandleGraph with a copy of graph where nodes and edges have
// been duplicated in such a way as to eliminate cycles while preserving all paths
// up to a given minimum length. Input HandleGraph must have a single stranded orientation.
// Consider checking this property with has_single_stranded_orientation() before using.
// Returns a mapping from the node IDs of into to the node IDs in graph.
std::unordered_map<nid_t, nid_t> dagify(const HandleGraph* graph, MutableHandleGraph* into,
                                        size_t min_preserved_path_length);
}
}

#endif
