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
    
    /// Write the contents of this graph to an ostream. Makes sure to include a
    /// leading magic number.
    inline void serialize(std::ostream& out) const;
    
    /// Sets the contents of this graph to the contents of a serialized graph from
    /// an istream. The serialized graph must be from the same implementation of the
    /// HandleGraph interface as is calling deserialize(). Can only be called on an
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
    // Make sure our byte wrangling is likely to work
    static_assert(sizeof(char) * 4 == sizeof(uint32_t), "Char must be 8 bits");
    
    // Read the first 4 bytes. We keep them in an array because we might need to unget them.
    char magic_bytes[4];
    in.read(magic_bytes, 4);
    
    uint32_t magic_number = ntohl(*((uint32_t*) magic_bytes));
    if (magic_number != get_magic_number()) {
        // They don't look right for what we are loading.
        // This could be an old file, or we could have been given the wrong kind of thing to load.
        std::cerr << "warning [libhandlegraph]: Serialized handle graph does not appear to match deserialzation type." << std::endl;
        std::cerr << "warning [libhandlegraph]: It is either an old version or in the wrong format." << std::endl;
        std::cerr << "warning [libhandlegraph]: Attempting to load it anyway. Future releases will reject it!" << std::endl;
        
        // Put the characters back in reverse order.
        for (int i = 3; i >= 0; i--) {
            in.putback(magic_bytes[i]);
        }
        
        if (!in) {
            // The stream did not rewind right (or was already at EOF somehow)
            throw std::runtime_error("Error rewinding to load non-magic-prefixed SerializableHandleGraph");
        }
    }
    deserialize_members(in);
}
}

#endif
