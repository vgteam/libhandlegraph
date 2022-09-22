#ifndef HANDLEGRAPH_ALGORITHMS_DIJKSTRA_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_DIJKSTRA_HPP_INCLUDED

/**
 * \file dijkstra.hpp
 *
 * Definitions of Dijkstra's Algorithm utilities for traversing the graph closest-first.
 */

#include <unordered_set>

#include "handlegraph/handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Walk out from the given handle and visit all reachable handles
/// (including the start) in the graph once, in closest-first order,
/// accounting for sequence lengths. Walks right unless traverse_leftward
/// is specified, in which case it walks left. Distances are measured
/// between the outgoing side of the start node and the incoming side of
/// the target.
///
/// When the shortest distance to a handle has been determined, calls
/// reached_callback with that handle and the distance to it. Calls to
/// reached_callback will occur in ascending order of distance. The
/// reached_callback function must return true to continue the search, and
/// false to abort it early.
///
/// If prune is true, aborts the search by pruining out edges away from the
/// current node and continuing with the next node in the queue. If it is
/// false, the whole search stops as soon as the callback returns false.
///
/// Returns true if the search terminated normally, and false if it was
/// aborted.
bool dijkstra(const HandleGraph* g, handle_t start,
              std::function<bool(const handle_t&, size_t)> reached_callback,
              bool traverse_leftward = false, bool prune = false);

/// Same as the single-start version, except allows starting from multiple
/// handles, all at distance 0.
bool dijkstra(const HandleGraph* g, const std::unordered_set<handle_t>& starts,
              std::function<bool(const handle_t&, size_t)> reached_callback,
              bool traverse_leftward = false, bool prune = false);
                                                      
}
}

#endif
