#include "handlegraph/trivially_serializable.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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

void TriviallySerializable::serialize(std::ostream& out) const {
    if (out.rdbuf() == std::cout.rdbuf()) {
        // Assume we are using standard output
        out << std::flush;
        serialize(STDOUT_FILENO);
        out << std::flush;
    } else if (out.rdbuf() == std::cerr.rdbuf()) {
        // Assume we are using standard error
        out << std::flush;
        serialize(STDERR_FILENO);
        out << std::flush;
    } else {
        // Just call the base class version for streams. We implemented the
        // stream IO to support it.
        Serializable::serialize(out);
    }
}

void TriviallySerializable::serialize(std::ostream& out) {
    // Same as above but non-const
    if (out.rdbuf() == std::cout.rdbuf()) {
        // Assume we are using standard output
        out << std::flush;
        serialize(STDOUT_FILENO);
        out << std::flush;
    } else if (out.rdbuf() == std::cerr.rdbuf()) {
        // Assume we are using standard error
        out << std::flush;
        serialize(STDERR_FILENO);
        out << std::flush;
    } else {
        // Just call the base class version for streams. We implemented the
        // stream IO to support it.
        Serializable::serialize(out);
    }
}

void TriviallySerializable::deserialize(std::istream& in) {
    if (in.rdbuf() == std::cin.rdbuf()) {
        // Assume we are using standard input
        deserialize(STDIN_FILENO);
    } else {
        // Just call the base class version for streams. We implemented the
        // stream IO to support it.
        Serializable::deserialize(in);
    }
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

}


