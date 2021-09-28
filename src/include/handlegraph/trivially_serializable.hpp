#ifndef HANDLEGRAPH_TRIVIALLY_SERIALIZABLE_HPP_INCLUDED
#define HANDLEGRAPH_TRIVIALLY_SERIALIZABLE_HPP_INCLUDED

/** \file 
 * Defines an interface for objects that us the same representation in memory and on disk. 
 */

#include "handlegraph/serializable.hpp"

#include <iostream>
#include <limits>
#include <functional>

namespace handlegraph {

/**
 * Interface for objects that can use identical in-memory and serialized representations.
 *
 * The representation begins with the serialized 4-byte magic number, followed
 * by user data. Length is implicit in either stream length or file size, and
 * can be grown.
 *
 * If serialization or deserialization throws, it is safe to destroy the
 * object, but not to do anything else with it.
 *
 * Modifying a file that an object has been loaded from or saved to, either on
 * disk or through another associated object, is undefined behavior.
 * To prevent modifications to an object from modifying the last file loaded or
 * saved to, use dissociate().
 */
class TriviallySerializable : public Serializable {

public:
   
    /**
     * Make a new TriviallySerializable.
     * Should really only be called by constructors of inheriting types.
     */
    TriviallySerializable() = default;
   
    /**
     * Destroy a TriviallySerializable object and any associated memory mappings.
     */
    virtual ~TriviallySerializable() = default;
    
    /**
     * Break the write-back link between this object and the file it was loaded
     * from, if any. Future modifications to the object will not affect the
     * file, although future modifications to the file may still affect the
     * object.
     */ 
    virtual void dissociate() = 0;
    
    // New TriviallySerializable feature: IO to file descriptors
    // If you want to do this with a normal Serializable, you will need a
    // function that sniffs if this interrface is supported and if not plugs in
    // your own FD-to-C++-stream wrapper. We don't include one in
    // libhandlegraph to avoid dependencies and duplicate code with libvgio.
    
    //Serialize as blocks of data shown to the given function. The pointer must not be null.
    virtual void serialize(const std::function<void(const void*, size_t)>& iteratee) const = 0;
    
    /// Write the contents of this object to an open file descriptor. Makes
    /// sure to include a leading magic number. If the file is a normal file,
    /// future modifications to the object will affect the file until
    /// dissociate() is called or another normal file is associated.
    ///
    /// Assumes that the file entirely belongs to this object.
    virtual void serialize(int fd) = 0;
    
    /// Sets the contents of this object to the contents of a serialized object
    /// from an open file descriptor. The serialized object must be from the
    /// same implementation of this interface as is calling deserialize(). Can
    /// only be called on an empty object  If the file is a normal writeable
    /// file, future modifications to the object will affect the file until
    /// dissociate() is called or another normal file is associated.
    ///
    /// Assumes that the file entirely belongs to this object.
    virtual void deserialize(int fd) = 0;
    
    //////////////////////////////////////////////////////////////////////////
    
    // Implementation doesn't need to provide anything below here; these all
    // call serialize_members() and deserialize_members(), or serialize(int)
    // and deserialize(int), or serialize(function)  as appropriate. 
    // But they can still be overridden if needed.

    /// Write the contents of this object to an open file descriptor. Makes
    /// sure to include a leading magic number.
    ///
    /// Assumes that the file entirely belongs to this object.
    virtual void serialize(int fd) const;

    // Interface takes control of serialization to and from named files and
    // routes it through file descriptor functions.

    /// Write the contents of this object to a named file. Makes sure to
    /// include a leading magic number. Does not affect any existing write-back
    /// links.
    virtual void serialize(const std::string& filename) const;
    /// Write the contents of this object to a named file. Makes sure to
    /// include a leading magic number. If the file is nonexistent or a normal
    /// file, future modifications to the object will affect the file until
    /// dissociate() is called or another normal file is associated.
    virtual void serialize(const std::string& filename);
    /// Sets the contents of this object to the contents of a serialized object
    /// from a file. The serialized object must be from the same implementation
    /// of this interface as is calling deserialize(). Can only be called on an
    /// empty object. If the file is a normal writeable file, future
    /// modifications to the object will affect the file until dissociate() is
    /// called or another normal file is associated.
    virtual void deserialize(const std::string& filename);
    
    // Interface also takes control of stream serialization and deserialization
    // that match standard input, output, and error, and delegate to
    // appropriate file descriptors.
    
    /// Dump the magic number and user data to the given stream. Does not
    /// affect any backing file link.
    virtual void serialize(std::ostream& out) const;
    /// Dump the magic number and user data to the given stream. Does not
    /// affect any backing file link.
    virtual void serialize(std::ostream& out);
    /// Sets the contents of this object to the contents of a serialized object
    /// from an istream. The serialized object must be from the same
    /// implementation of the interface as is calling deserialize(). Can only
    /// be called on an empty object.
    virtual void deserialize(std::istream& in);

protected:

    /// Helper to open a file descriptor with error checking.
    int open_fd(const std::string& filename) const;
    
    /// Helper to close a file descriptor with error checking.
    void close_fd(int fd) const;
};


}

#endif
