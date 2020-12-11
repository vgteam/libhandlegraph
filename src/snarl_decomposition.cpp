#include "handlegraph/snarl_decomposition.hpp"

#include <vector>

/** \file snarl_decomposition.cpp
 * Implement SnarlDecomposition methods.
 */

namespace handlegraph {

bool SnarlDecomposition::for_each_tippy_child_impl(const net_handle_t& parent, const std::function<bool(const net_handle_t&)>& iteratee) const {
    // This default implementation just scans for tips.
    // Should be overridden by one that knows where the tips, if any, are.
    for_each_child(parent, [&](const net_handle_t& child) {
        // Look at each child
        return for_each_traversal(child, [&](const net_handle_t child_trav) {
            // Look at each realizable traversal
            if (starts_at_tip(child_trav)) {
                // If it starts with a tip, show it to the iteratee and possibly stop.
                return iteratee(child_trav);
            } else {
                // Otherwise keep looking.
                return true;
            }
        });
    });
}

bool SnarlDecomposition::for_each_traversal_start_impl(const net_handle_t& traversal, const std::function<bool(const net_handle_t&)>& iteratee) const {
    switch (starts_at(traversal)) {
    case START:
        // Start bound, facing in
        return iteratee(get_bound(traversal, false, true));
        break;
    case END:
        // End bound, facing in
        return iteratee(get_bound(traversal, true, true));
        break;
    case TIP:
        // All tips
        return for_each_tippy_child(traversal, iteratee);
        break;
    }
}

bool SnarlDecomposition::for_each_traversal_end_impl(const net_handle_t& traversal, const std::function<bool(const net_handle_t&)>& iteratee) const {
    switch (ends_at(traversal)) {
    case START:
        // Start bound, facing out
        return iteratee(get_bound(traversal, false, false));
        break;
    case END:
        // End bound, facing out
        return iteratee(get_bound(traversal, true, false));
        break;
    case TIP:
        // All tippy children, flipped to end at tips
        return for_each_tippy_child(traversal, [&](const net_handle_t& child) {
            return iteratee(flip(child));
        });
        break;
    }
}

}


