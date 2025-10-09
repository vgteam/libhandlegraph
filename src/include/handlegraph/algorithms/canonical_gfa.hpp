#ifndef HANDLEGRAPH_ALGORITHMS_CANONICAL_GFA_INCLUDED
#define HANDLEGRAPH_ALGORITHMS_CANONICAL_GFA_INCLUDED

#include <iostream>

#include "handlegraph/handle_graph.hpp"

namespace handlegraph {
namespace algorithms {

//-----------------------------------------------------------------------------

/**
 * Writes the given HandleGraph to the given output stream in a canonical GFA
 * format. This format can be used for computing stable graph names using an
 * appropriate hashing function (see https://github.com/jltsiren/pggname).
 *
 * If integer_ids is true, node identifiers are interpreted as integers and
 * sorted in numerical order. Otherwise the identifiers are interpreted as
 * strings and sorted in lexicographic order.
 */
void canonical_gfa(const HandleGraph& graph, std::ostream& out, bool integer_ids);

 //-----------------------------------------------------------------------------

} // namespace algorithms
} // namespace handlegraph

#endif // HANDLEGRAPH_ALGORITHMS_CANONICAL_GFA_INCLUDED
