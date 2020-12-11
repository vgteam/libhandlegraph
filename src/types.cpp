#include "handlegraph/types.hpp"

#include "handlegraph/util.hpp"

/** \file types.cpp
 * Implement operators for libhandlegraph value types
 */

namespace handlegraph {

/// Define equality on handles
bool operator==(const handle_t& a, const handle_t& b) {
    return as_integer(a) == as_integer(b);
}

/// Define inequality on handles
bool operator!=(const handle_t& a, const handle_t& b) {
    return as_integer(a) != as_integer(b);
}

/// Define equality on path handles
bool operator==(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) == as_integer(b);
}

/// Define inequality on path handles
bool operator!=(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) != as_integer(b);
}

/// Define equality on step handles
bool operator==(const step_handle_t& a, const step_handle_t& b) {
    return as_integers(a)[0] == as_integers(b)[0] && as_integers(a)[1] == as_integers(b)[1];
}

/// Define inequality on step handles
bool operator!=(const step_handle_t& a, const step_handle_t& b) {
    return !(a == b);
}

/// Define equality on net handles
bool operator==(const net_handle_t& a, const net_handle_t& b) {
    return as_integers(a)[0] == as_integers(b)[0] && as_integers(a)[1] == as_integers(b)[1] && as_integers(a)[2] == as_integers(b)[2];
}

/// Define inequality on net handles
bool operator!=(const net_handle_t& a, const net_handle_t& b) {
    return !(a == b);
}

}


