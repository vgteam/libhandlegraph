#ifndef HANDLEGRAPH_INTERNAL_HANDLE_OPERATORS_HPP_INCLUDED
#define HANDLEGRAPH_INTERNAL_HANDLE_OPERATORS_HPP_INCLUDED

/** \file
 * Defines and user-facing operators to work with handles.
 */

#include "handlegraph/internal/handle_types.hpp"
#include "handlegraph/internal/handle_helper.hpp"

namespace handlegraph {

/// Define equality on handles
inline bool operator==(const handle_t& a, const handle_t& b) {
    return as_integer(a) == as_integer(b);
}

/// Define inequality on handles
inline bool operator!=(const handle_t& a, const handle_t& b) {
    return as_integer(a) != as_integer(b);
}

/// Define equality on path handles
inline bool operator==(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) == as_integer(b);
}

/// Define inequality on path handles
inline bool operator!=(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) != as_integer(b);
}

/// Define equality on occurrence handles
inline bool operator==(const occurrence_handle_t& a, const occurrence_handle_t& b) {
    return as_integers(a)[0] == as_integers(b)[0] && as_integers(a)[1] == as_integers(b)[1];
}

/// Define inequality on occurrence handles
inline bool operator!=(const occurrence_handle_t& a, const occurrence_handle_t& b) {
    return !(a == b);
}

}

#endif
