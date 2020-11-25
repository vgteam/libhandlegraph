#include "handlegraph/trivially_serializable.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>


/** \file trivially_serializable.cpp
 * Implement TriviallySerializable class-serving methods
 */

namespace handlegraph {

TriviallySerializable::~TriviallySerializable() {
}

void TriviallySerializable::dissociate() {
}

void TriviallySerializable::serialized_data_resize(size_t bytes) {
}

size_t TriviallySerializable::serialized_data_size() const {
}

char* TriviallySerializable::serialized_data() {
}

const char* TriviallySerializable::serialized_data() const {
}

void TriviallySerializable::serialize_members(std::ostream& out) const {
    // Serializable handle sthe magic number for us
    
    out.write(serialized_data(), serialized_data_size());
    // TODO: check error ourselves?
}

void TriviallySerializable::deserialize_members(std::istream& in) {
    // Serializable handles the magic number for us

    auto data_start = in.tellg();
    if (data_start != -1 && !in.fail()) {
        // Stream seems seekable
        in.seekg(0, in.end);
        auto data_end = is.tellg();
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
    Serializeable::serialize(out);
}

void TriviallySerializable::serialize(const std::string& filename) const {
    // Use the file descriptor version
    
    // Open a file descriptor
    int fd = ::open(filename.c_str(), ::O_RDWR | ::O_CREAT);
    if (fd == -1) {
        // Open failed; report a sensible problem.
        auto problem = ::errno;
        std::stringstream ss;
        ss << "Could not save to file " << filename << ": " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
    // Serialize to the file
    serialize(fd);
    
    // Close up the file
    if (::close(fd) != 0) {
        // An error happened closing
        auto problem = ::errno;
        std::stringstream ss;
        ss << "Could not close " << filename << ": " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
}

void TriviallySerializable::deserialize(std::istream& in) {
    // Just call the base class version for streams. We implemented the stream
    // IO to support it.
    Serializeable::deserialize(in);
}

void TriviallySerializable::deserialize(const std::string& filename) {

    // Use the file descriptor version
    
    // Try to open in read write mode
    int fd = ::open(filename.c_str(), ::O_RDWR);
    if (fd == -1) {
        // Try to open in read only mode instead.
        // Changes won't write back.
        fd = ::open(filename.c_str(), ::O_RDONLY); 
    }
    
    if (fd == -1) {
        // Open failed; report a sensible problem.
        auto problem = ::errno;
        std::stringstream ss;
        ss << "Could not load from file " << filename << ": " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }
    
    // Deserialize from the file
    deserialize(fd);
    
    // Close up the file
    if (::close(fd) != 0) {
        // An error happened closing
        auto problem = ::errno;
        std::stringstream ss;
        ss << "Could not close " << filename << ": " << ::strerror(problem);
        throw std::runtime_error(ss.str());
    }

}

void TriviallySerializable::serialize(int fd) const {
    if (serializedFD != -1) {
        // We have a file already. Maybe this is the same one.
        // If so, serializing would be a no-op.
        
        // Reserve buffers for file metadata
        ::stat our_stat;
        ::stat fd_stat;
        
        // Get info on the file we have open
        if (::fstat(serializedFD, &our_stat) != 0) {
            // It didn't work
            auto problem = ::errno;
            std::stringstream ss;
            ss << "Could not interrogate mapped file: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        // Get info on the file we want to use
        if (::fstat(fd, &fd_stat) != 0) {
            // It didn't work
            auto problem = ::errno;
            std::stringstream ss;
            ss << "Could not interrogate destination file: " << ::strerror(problem);
            throw std::runtime_error(ss.str());
        }
        
        if (our_stat.st_dev == fd_stat.st_dev && our_stat.st_ino == fd_stat.st_ino) {
            // These two file descriptors do indeed refer to the same file.
            // We don't have to do anything.
            return;
        }
    }
    
    // Otherwise, we need to write ourselves out to the new file.
    // And we need to handle the magic number ourselves.
    
    // TODO: For non-const, make sure we have some bytes allocated and the magic number there.
    // TODO: For const, specify that we don't change the backing mapping.
    // TODO: Funnel const and non-const down to some of the same code.
    
    // We first try to truncate it to the right length
    if (::ftruncate(fd, serializedLength) != 0) {
        // One reason we might not be able to truncate is that this is a stream
        // and not a normal file.
        // Try dumping to it. We assume that if it's not a normal file we'll
        // still fail.
        
        // Track how much of our data we have written so far.
        size_t written_bytes = 0;
        
        while (written_bytes < serializedLength) {
            // Until all the bytes are written
            
            // Try writing all the remaining bytes.
            auto written_now = ::write(fd, (const void*)(serialized_data() + written_bytes), serializedLength - written_bytes);
            
            if (written_now == -1) {
                // We encountered an error writing.
                auto problem = ::errno;
                std::stringstream ss;
                ss << "Could not write to destination file: " << ::strerror(problem);
                throw std::runtime_error(ss.str());
            }
            
            // Record how many we wrote
            written_bytes += written_now;
        }
        
        // Now we're done!
        return;
    }
    
    // We managed to truncate to the right length, so this must be a seekable
    // normal file.
    
    // The simple way to do this would be to dissociate from the old file, so
    // everything is in memory, and then save to and associate the new file.
    // 
    // We're not doing that.
    //
    // Instead, we're going to map the new file, memcpy to the new mapping, and
    // unmap the old file.

    void* new_mapping = ::mmap(nullptr, serializedLength, ::PROT_READ | ::PROT_WRITE, ::MAP_SHARED, fd, 0);

}

void TriviallySerializable::deserialize(int fd) {
}

}


