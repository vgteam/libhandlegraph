#ifndef HANDLEGRAPH_SERIALIZABLE_HANDLE_GRAPH_HPP_INCLUDED
#define HANDLEGRAPH_SERIALIZABLE_HANDLE_GRAPH_HPP_INCLUDED

/** \file 
 * Defines the base SerializableHandleGraph interface.
 */

#include <iostream>
#include <arpa/inet.h>

namespace handlegraph {

/*
 * Defines an interface for serialization and deserialization for handle graphs,
 * which can be co-inherited alongside HandleGraph.
 */
class SerializableHandleGraph {
    
public:

    virtual ~SerializableHandleGraph() = default;
    
    /// Returns a number that is specific to the serialized implementation for type
    /// checking. Does not depend on the contents of any particular instantiation
    /// (i.e. behaves as if static, but cannot be static and virtual).
    virtual uint32_t get_magic_number() const = 0;
    
    /// Write the contents of this graph to an ostream.
    inline void serialize(std::ostream& out) const;
    
    /// Sets the contents of this graph to the contents of a serialized graph from
    /// an istream. The serialized graph must be from the same implementation of the
    /// HandleGraph interface as is calling deserialize(). Can only be called by an
    /// empty graph.
    inline void deserialize(std::istream& in);
        
protected:
    
    /// Underlying implementation for "serialize" method
    virtual void serialize_members(std::ostream& out) const = 0;
    
    /// Underlying implementation to "deserialize" method
    virtual void deserialize_members(std::istream& in) = 0;
    
};





/**
 * Inline implementations
 */
        
inline void SerializableHandleGraph::serialize(std::ostream& out) const {
    uint32_t magic_number = htonl(get_magic_number());
    out.write((char*) &magic_number, sizeof(magic_number) / sizeof(char));
    serialize_members(out);
}

inline void SerializableHandleGraph::deserialize(std::istream& in) {
    uint32_t magic_number;
    in.read((char*) &magic_number, sizeof(magic_number) / sizeof(char));
    magic_number = ntohl(magic_number);
    if (magic_number != get_magic_number()) {
        throw std::runtime_error("error: Serialized handle graph does not match deserialzation type.");
    }
    deserialize_members(in);
}
}

#endif
