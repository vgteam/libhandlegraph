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
using SerializableHandleGraph = Serializable;

}

#endif
