#ifndef HANDLEGRAPH_SERIALIZABLE_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_SERIALIZABLE_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the base SerializableHandleGraph interface.
 */

#include "handlegraph/serializable.hpp"

namespace handlegraph {

/*
 * Defines an interface for serialization and deserialization for handle graphs,
 * which can be co-inherited alongside HandleGraph.
 */
class SerializableHandleGraph : public Serializable {
};
// Note that this needs to be a real class and not just a using because the
// libbdsg Python bindings want to bind it with "class" in front of it.

}

#endif
