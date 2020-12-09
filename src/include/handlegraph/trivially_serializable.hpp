#ifndef HANDLEGRAPH_TRIVIALLY_SERIALIZABLE_HPP_INCLUDED
#define HANDLEGRAPH_TRIVIALLY_SERIALIZABLE_HPP_INCLUDED

/** \file 
 * Defines an interface for objects that us the same representation in memory and on disk. 
 */

#include "handlegraph/serializable.hpp"

#include <iostream>
#include <limits>

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
     * Destroy a TriviallySerializable object and any associated memory mapping.
     */
    virtual ~TriviallySerializable();
    
    // We are not copyable or movable.
    // TODO: implement copy by dissociating backing storage.
    // TODO: Move should be easy.
    TriviallySerializable& operator=(const TriviallySerializable& other) = delete;
    TriviallySerializable& operator=(TriviallySerializable&& other) = delete;
    TriviallySerializable(const TriviallySerializable& other) = delete;
    TriviallySerializable(TriviallySerializable&& other) = delete;
    
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
     * Should be called by all mutable implementations when more (or fewer)
     * bytes are needed. The magic number size does not count against the total
     * length.
     *
     * After this is called, serialized_data() may return a different address.
     * Old pointers into user data are invalidated.
     *
     * Untouched pages will not reserve any memory.
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
    char* serialized_data();
    
    /**
     * Get the memory address of our serialized representation.
     * Returns nullptr if we have not ever been loaded or resized.
     *
     * Should be used by all implementations to get at their data.
     * The magic number is hidden.
     */
    const char* serialized_data() const;
    
    
    /// Final implementation of serialize_members for streams that dumps the
    /// data.
    void serialize_members(std::ostream& out) const final;
    
    /// Final implementation of deserialize_members for streams that loads the
    /// data.
    void deserialize_members(std::istream& in) final;
    

public:

    // Don't let implementations take control of serialize and deserialize by
    // not making our overrides virtual.

    /// Dump the magic number and user data to the given stream. Does not
    /// affect any backing file link.
    void serialize(std::ostream& out) const final;
    /// Dump the magic number and user data to the given stream. Does not
    /// affect any backing file link.
    void serialize(std::ostream& out) final;
    /// Write the contents of this object to a named file. Makes sure to
    /// include a leading magic number.
    void serialize(const std::string& filename) const final;
    /// Write the contents of this object to a named file. Makes sure to
    /// include a leading magic number. If the file is nonexistent or a normal
    /// file, future modifications to the object will affect the file until
    /// dissociate() is called or another normal file is associated.
    void serialize(const std::string& filename) final;
    
    /// Sets the contents of this object to the contents of a serialized object
    /// from an istream. The serialized object must be from the same
    /// implementation of the interface as is calling deserialize(). Can only
    /// be called on an empty object.
    void deserialize(std::istream& in) final;
    /// Sets the contents of this object to the contents of a serialized object
    /// from a file. The serialized object must be from the same implementation
    /// of this interface as is calling deserialize(). Can only be called on an
    /// empty object. If the file is a normal writeable file, future
    /// modifications to the object will affect the file until dissociate() is
    /// called or another normal file is associated.
    void deserialize(const std::string& filename) final;
    
    // New TriviallySerializable feature: IO to file descriptors
    // If you want to do this with a normal Serializable, you will need a
    // function that sniffs if this interrface is supported and if not plugs in
    // your own FD-to-C++-stream wrapper. We don't include one in
    // libhandlegraph to avoid dependencies and duplicate code with libvgio.
   
    /// Write the contents of this object to an open file descriptor. Makes
    /// sure to include a leading magic number.
    ///
    /// Assumes that the file entirely belongs to this object.
    void serialize(int fd) const;
    /// Write the contents of this object to an open file descriptor. Makes
    /// sure to include a leading magic number. If the file is a normal file,
    /// future modifications to the object will affect the file until
    /// dissociate() is called or another normal file is associated.
    ///
    /// Assumes that the file entirely belongs to this object.
    void serialize(int fd);
    
    /// Sets the contents of this object to the contents of a serialized object
    /// from an open file descriptor. The serialized object must be from the
    /// same implementation of this interface as is calling deserialize(). Can
    /// only be called on an empty object  If the file is a normal writeable
    /// file, future modifications to the object will affect the file until
    /// dissociate() is called or another normal file is associated.
    ///
    /// Assumes that the file entirely belongs to this object.
    void deserialize(int fd);

private:
    /// How many bytes of associated data are there?
    /// This includes the magic number!
    /// Might not be accuate if serialization/deserialization throws.
    size_t serializedLength = 0;
    /// Where is it in memory?
    /// This includes the magic number!
    void* serializedData = nullptr;
    /// What file descriptor do we have to the file for resizing it?
    /// Will be -1 if we don't have one.
    int serializedFD = -1;
    /// Even if serializedFD is -1, our mapping may actually be to a file, mapped privately.
    /// We can't necessarily safely grow a mapping with mremap if it's to a smaller file.
    /// This holds the size of the file our mapping is to, or
    /// std::numeric_limits<size_t>::max() if the mapping isn't to a file.
    size_t mappingFileSize = std::numeric_limits<size_t>::max();
    
    /// How big is the magic number?
    static constexpr int MAGIC_SIZE = sizeof(uint32_t) / sizeof(char);
    
    /// Helper function to set up output mappings to new files.
    void* serialize_and_get_mapping(int fd) const;
    
    /// Helper to open a file descriptor with error checking.
    int open_fd(const std::string& filename) const;
    
    /// Helper to close a file descriptor with error checking.
    void close_fd(int fd) const;
};


}

#endif
