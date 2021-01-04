#ifndef HANDLEGRAPH_ALGORITHMS_APPLY_ORIENTATIONS_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_APPLY_ORIENTATIONS_HPP_INCLUDED

/**
 * \file apply_orientations.hpp
 *
 * Defines an algorithm for applying orientations in bulk.
 */

#include "handlegraph/mutable_handle_graph.hpp"

#include <vector>
#include <unordered_set>

namespace handlegraph {
namespace algorithms {
    
/// Modifies underlying graph so that any node whose handle is given in the reverse orientation
/// is flipped so that all locally forward orientations match the orientation of the provided handles.
/// Returns a set of IDs for nodes that were flipped. Invalid if vector contains multiple handles to
/// the same node. May change the ordering of the underlying graph.
std::unordered_set<nid_t> apply_orientations(MutableHandleGraph* graph, const std::vector<handle_t>& orientations);

}
}

#endif
