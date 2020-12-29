/**
 * \file find_shortest_paths.cpp
 *
 * Implementation for the find_shortest_paths algorithm.
 */
 
#include "handlegraph/algorithms/find_shortest_paths.hpp"
#include "handlegraph/algorithms/dijkstra.hpp"

namespace handlegraph {
namespace algorithms {

using namespace std;

unordered_map<handle_t, size_t>  find_shortest_paths(const HandleGraph* g, handle_t start,
                                                     bool traverse_leftward) {

    // This is the minimum distance to each handle
    unordered_map<handle_t, size_t> distances;
    
    dijkstra(g, start, [&](const handle_t& current, size_t distance) {
        // Record handle's distance
        distances[current] = distance;
        
        // Always keep going
        return true;
    }, traverse_leftward);
    
    return distances;

}

    
}
}
