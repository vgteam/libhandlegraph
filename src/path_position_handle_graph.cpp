#include "handlegraph/path_position_handle_graph.hpp"

#include <vector>

/** \file path_position_handle_graph.cpp
 * Implement PathPositionHandleGraph methods.
 */

namespace handlegraph {

bool PathPositionHandleGraph::for_each_step_position_on_handle(const handle_t& handle,
                                                               const std::function<bool(const step_handle_t&, const bool&, const size_t&)>& iteratee) const {
    return for_each_step_on_handle_impl(handle, [&](const step_handle_t& step) {
        return iteratee(step, get_is_reverse(get_handle_of_step(step)) != get_is_reverse(handle), get_position_of_step(step));
    });
}
    
}


