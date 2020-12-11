#include "handlegraph/serializable.hpp"

#include <fstream>
#include <arpa/inet.h>

/** \file serializable.cpp
 * Implement Serializable default methods
 */

namespace handlegraph {

void Serializable::serialize(std::ostream& out) const {
    uint32_t magic_number = htonl(get_magic_number());
    out.write((char*) &magic_number, sizeof(magic_number) / sizeof(char));
    serialize_members(out);
}

void Serializable::serialize(std::ostream& out) {
    ((const Serializable*) this)->serialize(out);
}

void Serializable::serialize(const std::string& filename) const {
    std::ofstream out(filename);
    serialize(out);
}

void Serializable::serialize(const std::string& filename) {
    ((const Serializable*) this)->serialize(filename);
}


void Serializable::deserialize(std::istream& in) {
    // Make sure our byte wrangling is likely to work
    static_assert(sizeof(char) * 4 == sizeof(uint32_t), "Char must be 8 bits");
    
    // Read the first 4 bytes. We keep them in an array because we might need to unget them.
    char magic_bytes[4];
    in.read(magic_bytes, 4);
    
    uint32_t magic_number = ntohl(*((uint32_t*) magic_bytes));
    if (magic_number != get_magic_number()) {
        // They don't look right for what we are loading.
        // This could be an old file, or we could have been given the wrong kind of thing to load.
        std::cerr << "warning [libhandlegraph]: Serialized object does not appear to match deserialzation type." << std::endl;
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

void Serializable::deserialize(const std::string& filename) {
    std::ifstream in(filename);
    deserialize(in);
}

}


