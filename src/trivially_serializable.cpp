#include "handlegraph/trivially_serializable.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <cassert>
#include <sstream>


/** \file trivially_serializable.cpp
 * Implement TriviallySerializable class-serving methods
 */

namespace handlegraph {

TriviallySerializable::~TriviallySerializable() {
    // Don't check errors here!
    if (serializedData != nullptr) {
        // Length must be correct
        ::munmap(serializedData, serializedLength);
        serializedData = nullptr;
        serializedLength = 0;
    }
    if (serializedFD != -1) {
        ::close(serializedFD);
        serializedFD = -1;
    }
}

void TriviallySerializable::dissociate() {
    if (serializedFD != -1) {
        // If we have a file we are writing back to
        
        // We really should have a mapping.
        assert(serializedData != nullptr);
        
        // Remap as private (just over the existing mapping, without unmapping)
        void* new_mapping = ::mmap(serializedData, serializedLength, PROT_READ | PROT_WRITE, MAP_PRIVATE, serializedFD, 0);
        
        if (new_mapping == nullptr) {
            // Mapping failed!
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not remap private: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        if (new_mapping != serializedData) {
            // The data had to move
            
            // So we need to get rid of the old one
            if (::munmap(serializedData, serializedLength) != 0) {
                auto problem = errno;
                std::stringstream ss;
                ss << "Could not remove old mapping: " << ::strerror(problem);
                // Try to unmap the new mapping so we don't leave it behind
                // after the exception is caught.
                ::munmap(new_mapping, serializedLength);
                throw std::runtime_error(ss.str());
            }
            
            serializedData = new_mapping;
        }
        
        // And close the file
        close_fd(serializedFD);
        serializedFD = -1;
    }
}

void TriviallySerializable::serialized_data_resize(size_t bytes) {
    
    // Determine new total size including magic number
    size_t new_serialized_length = MAGIC_SIZE + bytes;
    
    if (serializedData == nullptr) {
        // We need to initially allocate, and populate the magic number.
        
        // No file ought to be associated
        assert(serializedFD == -1);
        
        serializedData = ::mmap(nullptr, new_serialized_length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (serializedData == nullptr) {
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not create anonymous mapping: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        // Fill in the magic number
        *((uint32_t*)serializedData) = htonl(get_magic_number());
    } else if (new_serialized_length == serializedLength) {
        // Some data is already allocated, and it is already the right size.
        // Do nothing.
        return;
    } else {
        // Some data is already allocated, and it is the wrong size.
        
        // See if we have a file
        if (serializedFD != -1) {
            // If so, we need to lengthen or shorten it.
            // Supposedly you can get undefined behavior with ftruncate and
            // partial mapped pages, but the man page at
            // <https://linux.die.net/man/3/ftruncate> doesn't back that up.
            
            if (new_serialized_length > serializedLength) {
                // Lengthening, so truncate and then remap, allowing to move
                if (::ftruncate(serializedFD, new_serialized_length) != 0) {
                    auto problem = errno;
                    std::stringstream ss;
                    ss << "Could not grow mapped file: " << ::strerror(problem);
                    throw std::runtime_error(ss.str());
                }
                
                void* new_mapping = ::mremap(serializedData, serializedLength, new_serialized_length, MREMAP_MAYMOVE);
                if (new_mapping == nullptr) {
                    auto problem = errno;
                    std::stringstream ss;
                    ss << "Could not grow mapping: " << ::strerror(problem);
                    throw std::runtime_error(ss.str());
                }
                
                // Commit to object
                serializedData = new_mapping;
                serializedLength = new_serialized_length;
            } else {
                // Shortening, so remap in place and then truncate.
                void* new_mapping = ::mremap(serializedData, serializedLength, new_serialized_length, 0);
                if (new_mapping == nullptr) {
                    auto problem = errno;
                    std::stringstream ss;
                    ss << "Could not shrink mapping: " << ::strerror(problem);
                    throw std::runtime_error(ss.str());
                }
                // It's not allowed to move,
                assert(new_mapping == serializedData);
                
                // Save the length change now so we can unmap on exception
                serializedLength = new_serialized_length;
                
                if (::ftruncate(serializedFD, new_serialized_length) != 0) {
                    auto problem = errno;
                    std::stringstream ss;
                    ss << "Could not shrink mapped file: " << ::strerror(problem);
                    throw std::runtime_error(ss.str());
                }
            }
        } else {
            // Otherwise, we have no file.
            // Just mremap the anonymous mapping, allowing to move.
            void* new_mapping = ::mremap(serializedData, serializedLength, new_serialized_length, MREMAP_MAYMOVE);
            if (new_mapping == nullptr) {
                auto problem = errno;
                std::stringstream ss;
                ss << "Could not resize mapping: " << ::strerror(problem);
                throw std::runtime_error(ss.str());
            }
            
            // Commit to object
            serializedData = new_mapping;
            serializedLength = new_serialized_length;
        }
    }
}

size_t TriviallySerializable::serialized_data_size() const {
    if (serializedData != nullptr) {
        assert(serializedLength > MAGIC_SIZE);
        return serializedLength - MAGIC_SIZE;
    }
    return 0;
}

char* TriviallySerializable::serialized_data() {
    if (serializedData != nullptr) {
        return ((char*)serializedData) + MAGIC_SIZE;
    }
    return nullptr;
}

const char* TriviallySerializable::serialized_data() const {
    if (serializedData != nullptr) {
        return ((const char*)serializedData) + MAGIC_SIZE;
    }
    return nullptr;
}

void TriviallySerializable::serialize_members(std::ostream& out) const {
    // Serializable handle sthe magic number for us
    
    out.clear();
    out.write(serialized_data(), serialized_data_size());
    if (out.fail()) {
        throw std::runtime_error("Could not write " + std::to_string(serialized_data_size()) + " bytes to stream");
    }
}

void TriviallySerializable::deserialize_members(std::istream& in) {
    // Serializable handles the magic number for us
    
    // We might have an implementation with a constructor that allocates some
    // memory already. That's fine; we'll clobber it.
    // We shouldn't be called when we've already read or written, but just to be safe...
    dissociate();

    auto data_start = in.tellg();
    if (data_start != -1 && !in.fail()) {
        // Stream seems seekable
        in.seekg(0, in.end);
        auto data_end = in.tellg();
        in.seekg(data_start);
        
        if (data_end != -1 && !in.fail()) {
            // All this seeking worked and we have the file length.
            // Resize and do one big read.
            serialized_data_resize(data_end - data_start);
            in.clear();
            in.read(serialized_data(), serialized_data_size());
            if (in.fail() || in.bad() || in.eof()) {
                throw std::runtime_error("Could not read " + std::to_string(serialized_data_size()) + " bytes from stream");
            }
            return;
        }
    }
    
    // Otherwise the stream wasn't seekable. Hopefully we're at the start.
    in.seekg(data_start);
    in.clear();
    
    // This tracks how many bytes of our data are actually used.
    size_t read_bytes = 0;
    
    // Start with a relatively large size, because big IO calls are fast and
    // untouched pages here are free.
    serialized_data_resize(1024 * 1024);
    
    while (!in.eof()) {
        if (read_bytes == serialized_data_size()) {
            // Make the buffer bigger because we have filled it.
            // Because untouched pages don't take any memory, we don't need to
            // worry about this getting too big really.
            serialized_data_resize(serialized_data_size() * 2);
        }
    
        // Try and read enough to fill the buffer.
        in.read(serialized_data() + read_bytes, serialized_data_size() - read_bytes);
        if (in.bad()) {
            throw std::runtime_error("Could not read from stream");
        }
        // We'll eventually EOF, probably in the middle of one of these reads.
        // Record how much we actually got this time.
        read_bytes += in.gcount();
    }
    
    // Now we have all the data. Shrink back to the size we actually got.
    serialized_data_resize(read_bytes);
    
    // Now we're done!
}

void TriviallySerializable::serialize(std::ostream& out) const {
    // Just call the base class version for streams. We implemented the stream
    // IO to support it.
    Serializable::serialize(out);
}

void TriviallySerializable::serialize(std::ostream& out) {
    // Do the same thing but non-const.
    // Ends up doing the same as above; we don't use the non-const hook here.
    Serializable::serialize(out);
}

// To let the const and non-const filename serialization implementations share
// code, we have some helpers

int TriviallySerializable::open_fd(const std::string& filename) const {
    // Open a file descriptor
    int fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        // Open failed; report a sensible problem.
        auto problem = errno;
        std::stringstream ss;
        ss << "Could not save to file " << filename << ": " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
    return fd;
}

void TriviallySerializable::close_fd(int fd) const {
    // Close up the file
    if (::close(fd) != 0) {
        // An error happened closing
        auto problem = errno;
        std::stringstream ss;
        ss << "Could not close FD: " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
}

void TriviallySerializable::serialize(const std::string& filename) const {
    int fd = open_fd(filename);
    
    // Serialize to the file, as const
    serialize(fd);
    
    close_fd(fd);
}

void TriviallySerializable::serialize(const std::string& filename) {
    int fd = open_fd(filename);
    
    // Serialize to the file, as non const
    serialize(fd);
    
    close_fd(fd);
}

void TriviallySerializable::deserialize(std::istream& in) {
    // Just call the base class version for streams. We implemented the stream
    // IO to support it.
    Serializable::deserialize(in);
}

void TriviallySerializable::deserialize(const std::string& filename) {

    // Use the file descriptor version
    
    // Try to open in read write mode
    int fd = ::open(filename.c_str(), O_RDWR);
    if (fd == -1) {
        // Try to open in read only mode instead.
        // Changes won't write back.
        fd = ::open(filename.c_str(), O_RDONLY); 
    }
    
    if (fd == -1) {
        // Open failed; report a sensible problem.
        auto problem = errno;
        std::stringstream ss;
        ss << "Could not load from file " << filename << ": " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
    // Deserialize from the file
    deserialize(fd);
    
    // Close up the file
    close_fd(fd);
}

void* TriviallySerializable::serialize_and_get_mapping(int fd) const {
    // We need to serialize and return the new maping (which may be the same as the current one, which may be null).
    // Then non-const FD serialize can adopt it.
    
    if (serializedFD != -1) {
        // We have a file already. Maybe this is the same one.
        // If so, serializing would be a no-op.
        
        // Reserve buffers for file metadata
        struct stat our_stat;
        struct stat fd_stat;
        
        // Get info on the file we have open
        if (::fstat(serializedFD, &our_stat) != 0) {
            // It didn't work
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not interrogate mapped file: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        // Get info on the file we want to use
        if (::fstat(fd, &fd_stat) != 0) {
            // It didn't work
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not interrogate destination file: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        if (our_stat.st_dev == fd_stat.st_dev && our_stat.st_ino == fd_stat.st_ino) {
            // These two file descriptors do indeed refer to the same file.
            // We don't have to do anything.
            return serializedData;
        }
    }
    
    // Otherwise, we need to write ourselves out to the new file.
    // And we need to handle the magic number ourselves.
    
    // Work out how many bytes we need to write, given that we might not have any real data yet.
    size_t output_length;
    uint32_t magic_buffer;
    const char* data_to_write;
    
    if (serializedData) {
        // We have data
        data_to_write = (const char*) serializedData;
        output_length = serializedLength;
    } else {
        // Just buffer the magic number on the stack.
        magic_buffer = htonl(get_magic_number());
        data_to_write = (const char*) &magic_buffer;
        output_length = MAGIC_SIZE;
    }
    
    // We first try to truncate it to the right length
    if (::ftruncate(fd, output_length) != 0) {
        // One reason we might not be able to truncate is that this is a stream
        // and not a normal file.
        // Try dumping to it. We assume that if it's not a normal file we'll
        // still fail.
        
        // Track how much of our data we have written so far.
        size_t written_bytes = 0;
        
        while (written_bytes < output_length) {
            // Until all the bytes are written
            
            // Try writing all the remaining bytes.
            auto written_now = ::write(fd, (const void*)(data_to_write + written_bytes), output_length - written_bytes);
            
            if (written_now == -1) {
                // We encountered an error writing.
                auto problem = errno;
                std::stringstream ss;
                ss << "Could not write to destination file: " << ::strerror(problem);
                throw std::runtime_error(ss.str());
            }
            
            // Record how many we wrote
            written_bytes += written_now;
        }
        
        // Now we're done!
        // Keep whatever (maybe null) mapping we had.
        return serializedData;
    }
    
    // We managed to truncate to the right length, so this must be a seekable
    // normal file.
    
    // TODO: We would like the new mapped memory to come into existence knowing it ought to match the old mapped memory.
    // TODO: work out a way to reflink here instead so the file can really be CoW.
    // Right now we will just do a big memcpy between disk-backed regions.
    
    // Make the mapping
    void* new_mapping = ::mmap(nullptr, output_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (new_mapping == nullptr) {
        auto problem = errno;
        std::stringstream ss;
        ss << "Could not map memory: " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
    // Fill it in from either our current mapping or the magic number on stack.
    ::memcpy(new_mapping, data_to_write, output_length);
    
    // Return it
    return new_mapping;
}

void TriviallySerializable::serialize(int fd) const {
    
    void* new_mapping = serialize_and_get_mapping(fd);
    
    // Now turn around and unmap the new mapping
    // We know it's either the size of our data, or the size of the magic number if we have no data.
    if (::munmap(new_mapping, serializedData ? serializedLength : MAGIC_SIZE) != 0) {
        // We encountered an error unmapping.
        auto problem = errno;
        std::stringstream ss;
        ss << "Could not unmap memory: " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }

}

void TriviallySerializable::serialize(int fd) {
    void* new_mapping = serialize_and_get_mapping(fd);
    
    // Now adopt the new mapping
    if (serializedData) {
        // Unmap the old one
        if (::munmap(serializedData, serializedLength) != 0) {
            // We encountered an error unmapping.
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not unmap memory: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        serializedData = nullptr;
    }
    
    if (serializedFD != -1) {
        // Close the old FD
        close_fd(serializedFD);
        serializedFD = -1;
    }
    // Adopt the new one via dup
    serializedFD = dup(fd);
    if (serializedFD == -1) {
        auto problem = errno;
        std::stringstream ss;
        ss << "Could not keep descriptor to mapped file: " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
}

void TriviallySerializable::deserialize(int fd) {
    
    if (serializedData != nullptr) {
        // We have data already. Unmap it.
        if (::munmap(serializedData, serializedLength) != 0) {
            // We encountered an error unmapping.
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not unmap memory: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        serializedData = nullptr;
    }
    if (serializedFD != -1) {
        // Close any old FD
        close_fd(serializedFD);
        serializedFD = -1;
    }

    auto file_length = ::lseek(fd, 0, SEEK_END);
    if (file_length != -1) {
        // We can probably seek in this probably normal file.
        // Go back to the start.
        if (::lseek(fd, 0, SEEK_SET) != 0) {
            // But we couldn't seek back. We messed it up!
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not seek back to start of file: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        // Now mmap
        serializedLength = file_length;
        serializedData = ::mmap(nullptr, file_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (serializedData == nullptr) {
            // Try mapping private if read-write and shared doesn't work.
            // Needs to be writeable or we will segfault if someone tries to modify us.
            serializedData = ::mmap(nullptr, file_length, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        }
        if (serializedData == nullptr) {
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not map memory: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        // Adopt the new FD via dup
        serializedFD = dup(fd);
        if (serializedFD == -1) {
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not keep descriptor to mapped file: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        // Validate magic number length and value.
        if (serializedLength < MAGIC_SIZE) {
            throw std::runtime_error("EOF in magic number");
        }
        // Just grab it from the start of our mapping.
        uint32_t observed_magic = ntohl(*(uint32_t*)serializedData);
        if (observed_magic != get_magic_number()) {
            throw std::runtime_error("Incorrect magic number " + std::to_string(observed_magic) + " should be " + std::to_string(get_magic_number()));
        }
        
        // Now we're done!
        return;
    }
    
    // If we get here, we have to handle the streaming case.
    // TODO: deduplicate with deserialize_members?
    
    // This tracks the number of bytes read in the last read operation.
    ssize_t current_bytes;
    
    // Read magic number
    uint32_t observed_magic;
    size_t magic_cursor = 0;
    do {
        // Get bytes
        current_bytes = ::read(fd, (void*)(((char*)&observed_magic) + magic_cursor), MAGIC_SIZE - magic_cursor);
        if (current_bytes < 0) {
            // An error occurred
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not read from FD stream: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        magic_cursor += current_bytes;
    } while (current_bytes > 0 && magic_cursor < MAGIC_SIZE);
    
    if (current_bytes == 0) {
        // We got an EOF in the magic number.
        throw std::runtime_error("EOF in magic number");
    }
    
    // Validate magic number
    observed_magic = ntohl(observed_magic);
    if (observed_magic != get_magic_number()) {
        // This sort of mismatch is now the programmer's problem.
        throw std::runtime_error("Incorrect magic number " + std::to_string(observed_magic) + " should be " + std::to_string(get_magic_number()));
    }
    
    // From here on we can just use the normal size and data functions we
    // present to users, but populating from an FD to a stream.
    
    // This tracks how many bytes of our data are actually used.
    size_t read_bytes = 0;
    
    // Start with a relatively large size, because big IO calls are fast and
    // untouched pages here are free.
    serialized_data_resize(1024 * 1024);
    
    do {
        if (read_bytes == serialized_data_size()) {
            // Make the buffer bigger because we have filled it.
            // Because untouched pages don't take any memory, we don't need to
            // worry about this getting too big really.
            serialized_data_resize(serialized_data_size() * 2);
        }
        
        // Get bytes
        current_bytes = ::read(fd, (void*)(serialized_data() + read_bytes), serialized_data_size() - read_bytes);
        if (current_bytes < 0) {
            // An error occurred
            auto problem = errno;
            std::stringstream ss;
            ss << "Could not read from FD stream: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        read_bytes += current_bytes;
    } while (current_bytes > 0);
    
    // Now we have all the data. Shrink back to the size we actually got.
    serialized_data_resize(read_bytes);
}

// TODO overall:
// EINTR handling

}


