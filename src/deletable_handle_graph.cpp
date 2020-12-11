#include "handlegraph/deletable_handle_graph.hpp"

/** \file deletable_handle_graph.cpp
 * Implement methods for DeletableHandleGraph.
 */

namespace handlegraph {

handle_t DeletableHandleGraph::truncate_handle(const handle_t& handle, bool trunc_left, size_t offset) {
    auto halves = divide_handle(handle, offset);
    if (trunc_left) {
        destroy_handle(halves.first);
        return halves.second;
    }
    else {
        destroy_handle(halves.second);
        return halves.first;
    }
}

}


