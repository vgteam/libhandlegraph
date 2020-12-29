#ifndef HANDLEGRAPH_ALGORITHMS_STRONGLY_CONNECTED_COMPONENTS_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_STRONGLY_CONNECTED_COMPONENTS_HPP_INCLUDED

#include <vector>
#include <unordered_set>

#include "handlegraph/handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Identify strongly connected components
std::vector<std::unordered_set<nid_t>> strongly_connected_components(const HandleGraph* g);
    
}
}

#endif
