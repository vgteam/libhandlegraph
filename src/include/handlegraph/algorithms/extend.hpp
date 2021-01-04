#ifndef HANDLEGRAPH_ALGORITHMS_EXTEND_HPP_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_EXTEND_HPP_INCLUDED

/**
 * \file extend.hpp
 *
 * Defines algorithm adding graph material from one graph into another
 */

#include "handlegraph/mutable_handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

/// Adds the non-duplicative nodes and edges from 'source' to 'into'. Assumes that
/// both graphs use the same ID space.
void extend(const HandleGraph* source, MutableHandleGraph* into);

}
}

#endif
