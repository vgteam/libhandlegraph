#ifndef HANDLEGRAPH_SERIALIZABLE_HPP_INCLUDED
#define HANDLEGRAPH_SERIALIZABLE_HPP_INCLUDED

/** \file 
 * Defines an interface for objects that are saveable and loadable. 
 */

#include <iostream>

namespace handlegraph {

/*
 * Defines an interface for serialization and deserialization. Can be
 * co-inherited alongside HandleGraph or other interfaces in the library.
 *
 * All serialized items begin with a 4-byte magic number. Implementations might
 * not be self-delimiting in a longer stream, and might not be safely
 * concatenateable.
 */
class Serializable {

public:

    virtual ~Serializable() = default;
    
    ////////////////////////////////////////
    // These methods should be implemented
    ////////////////////////////////////////
    
protected:

    /// Underlying implementation for "serialize" method
    virtual void serialize_members(std::ostream& out) const = 0;
    
    /// Underlying implementation to "deserialize" method
    virtual void deserialize_members(std::istream& in) = 0;
    
public:
    
    /// Returns a number that is specific to the serialized implementation for type
    /// checking. Does not depend on the contents of any particular instantiation
    /// (i.e. behaves as if static, but cannot be static and virtual).
    virtual uint32_t get_magic_number() const = 0;
    
    ////////////////////////////////////////
    // These methods can be overridden if serialization to files or file
    // descriptors can be accelerated relative to serialization to streams.
    ////////////////////////////////////////
    
    /// Write the contents of this object to an ostream. Makes sure to include a
    /// leading magic number.
    virtual void serialize(std::ostream& out) const;
    /// Write the contents of this object to a named file. Makes sure to include
    /// a leading magic number.
    virtual void serialize(const std::string& filename) const;
    
    /// Sets the contents of this object to the contents of a serialized object
    /// from an istream. The serialized object must be from the same
    /// implementation of the interface as is calling deserialize(). Can only
    /// be called on an empty object.
    virtual void deserialize(std::istream& in);
    /// Sets the contents of this object to the contents of a serialized object
    /// from a file. The serialized object must be from the same implementation
    /// of this interface as is calling deserialize(). Can only be called on an
    /// empty object.
    virtual void deserialize(const std::string& filename);
        
};

}

#endif
