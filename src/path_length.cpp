#include "handlegraph/algorithms/path_length.hpp"
#include "handlegraph/path_position_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

size_t get_path_length(const PathHandleGraph* graph, const path_handle_t& path) {
    // A path-position graph already knows this.
    if (const PathPositionHandleGraph* positional
        = dynamic_cast<const PathPositionHandleGraph*>(graph)) {
        return positional->get_path_length(path);
    }
    size_t length = 0;
    for (handle_t h : graph->scan_path(path)) {
        length += graph->get_length(h);
    }
    return length;
}

}
}
