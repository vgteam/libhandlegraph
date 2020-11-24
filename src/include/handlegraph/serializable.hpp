#ifndef HANDLEGRAPH_SERIALIZABLE_HPP_INCLUDED
#define HANDLEGRAPH_SERIALIZABLE_HPP_INCLUDED

/** \file 
 * Defines an interface for objects that are saveable and loadable. 
 */

#include <iostream>
#include <fstream>
#include <arpa/inet.h>

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
    // These methods can be overridden if serialization to files can be
    // accelerated relative to serialization to streams.
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

/**
 * Interface for objects that can use identical in-memory and serialized representations.
 *
 * The representation begins with the serialized 4-byte magic number, followed
 * by user data. Length is implicit in either stream length or file size, and
 * can be grown.
 */
class TriviallySerializable : public Serializable {

public:
    
    /**
     * Destroy a TriviallySerializable object and any associated memory mapping.
     */
    virtual ~TriviallySerializable();
    
    /**
     * Break the write-back link between this object and the file it was loaded
     * from, if any. Future modifications to the object will not affect the
     * file, although future modifications to the file may still affect the
     * object.
     *
     * After this is called, serialized_data() may return a different address.
     * Old pointers into user data are invalidated.
     */ 
    void dissociate();

protected:
    /**
     * Resize our serialized representation to the given number of bytes.
     * Modifies any backing on-disk file.
     *
     * Should be called by all mutable implementations when more bytes are
     * needed. The magic number size does not count against the total length.
     *
     * After this is called, serialized_data() may return a different address.
     * Old pointers into user data are invalidated.
     */
    void serialized_data_resize(size_t bytes);

    /**
     * Get the number of bytes of user data.
     * Does not count the magic number.
     */
    size_t serialized_data_size() const;

    /**
     * Get the memory address of our serialized representation.
     * Returns nullptr if we have not ever been loaded or resized.
     *
     * Should be used by all implementations to get at their data.
     * The magic number is hidden.
     */
    void* serialized_data();
    
    /**
     * Get the memory address of our serialized representation.
     * Returns nullptr if we have not ever been loaded or resized.
     *
     * Should be used by all implementations to get at their data.
     * The magic number is hidden.
     */
    const void* serialized_data() const;
    

public:
    /// Dump the magic number and dht user data to the given stream. Does not
    /// affect any backing file link.
    virtual void serialize(std::ostream& out) const;
    /// Write the contents of this object to a named file. Makes sure to
    /// include a leading magic number. If the file is nonexistent or a normal
    /// file, future modifications to the object will affect the file until
    /// dissociate() is called or another file is associated.
    virtual void serialize(const std::string& filename) const;
    
    /// Sets the contents of this object to the contents of a serialized object
    /// from an istream. The serialized object must be from the same
    /// implementation of the interface as is calling deserialize(). Can only
    /// be called on an empty object.
    virtual void deserialize(std::istream& in);
    /// Sets the contents of this object to the contents of a serialized object
    /// from a file. The serialized object must be from the same implementation
    /// of this interface as is calling deserialize(). Can only be called on an
    /// empty object  If the file is a normal file, future modifications to the
    /// object will affect the file until dissociate() is called or another
    /// file is associated.
    virtual void deserialize(const std::string& filename);

private:
    /// How many bytes of associated data are there?
    size_t serializedLength = 0;
    /// Where is it in memory?
    void* serializedData = nullptr;
    /// What file descriptor do we have to the file for resizing it?
    /// Will be 0 if we don't have one.
    int serializedFD = 0;
};


}

#endif
