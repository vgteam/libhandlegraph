#include "handlegraph/algorithms/apply_orientations.hpp"

namespace handlegraph {
namespace algorithms {

using namespace std;

unordered_set<nid_t> apply_orientations(MutableHandleGraph* graph, const vector<handle_t>& orientations) {
    
    // Track what we flip
    unordered_set<nid_t> flipped;
    for (const auto& handle : orientations) {
        if (graph->get_is_reverse(handle)) {
            // This needs to be flipped
            flipped.insert(graph->get_id(handle));
            // Flip it
            graph->apply_orientation(handle);
        }
    }
    return flipped;
}
}
}
