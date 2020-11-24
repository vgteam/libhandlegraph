#include "handlegraph/path_position_handle_graph.hpp"

/** \file ranked_handle_graph.cpp
 * Implement methods for RankedHandleGraph.
 */

namespace handlegraph {

/// Return the rank of a handle (ranks start at 1 and are dense, and each
/// orientation has its own rank). Handle ranks may not have anything to do
/// with node ranks.
size_t RankedHandleGraph::handle_to_rank(const handle_t& handle) const {
    // Display all the orientations forward then reverse, in node order, starting at 1.
    return id_to_rank(get_id(handle)) * 2 - 1 + (size_t) get_is_reverse(handle);
}

/// Return the handle with a given rank.
handle_t RankedHandleGraph::rank_to_handle(const size_t& rank) const {
    // 1 and 2 are node rank 0 forward and reverse, 3 and 4 are node rank 1 forward and reverse, etc.
    return get_handle(rank_to_id((rank - 1)/2 + 1), rank % 2 == 0);
}

}


