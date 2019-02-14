#ifndef HANDLEGRAPH_INTERNAL_HANDLE_HELPER_HPP_INCLUDED
#define HANDLEGRAPH_INTERNAL_HANDLE_HELPER_HPP_INCLUDED

/** \file
 * Tools for handle graph implementers to pack and unpack handles.
 */

#include <cassert>

namespace handlegraph {

//
// Handles
//

/// View a handle as an integer
inline uint64_t& as_integer(handle_t& handle) {
    return reinterpret_cast<uint64_t&>(handle);
}

/// View a const handle as a const integer
inline const uint64_t& as_integer(const handle_t& handle) {
    return reinterpret_cast<const uint64_t&>(handle);
}

/// View an integer as a handle
inline handle_t& as_handle(uint64_t& value) {
    return reinterpret_cast<handle_t&>(value);
}

/// View a const integer as a const handle
inline const handle_t& as_handle(const uint64_t& value) {
    return reinterpret_cast<const handle_t&>(value);
}


/// Define a way to pack an integer and an orientation bit into a handle_t.
struct number_bool_packing {

    /// Extract the packed integer
    inline static uint64_t unpack_number(const handle_t& handle) {
        return as_integer(handle) >> 1;
    }
    
    /// Extract the packed bit
    inline static bool unpack_bit(const handle_t& handle) {
        return as_integer(handle) & 1;
    }
    
    /// Pack up an integer and a bit into a handle
    inline static handle_t pack(const uint64_t& number, const bool& bit) {
        // Make sure the number doesn't use all the bits
        assert(number < (0x1ULL << 63));
        
        return as_handle((number << 1) | (bit ? 1 : 0));
    }
    
    /// Toggle the packed bit and return a new handle
    inline static handle_t toggle_bit(const handle_t& handle) {
        return as_handle(as_integer(handle) ^ 1);
    }
};

//
// Path handles
//

/// View a path handle as an integer
inline uint64_t& as_integer(path_handle_t& handle) {
    return reinterpret_cast<uint64_t&>(handle);
}

/// View a const path handle as a const integer
inline const uint64_t& as_integer(const path_handle_t& handle) {
    return reinterpret_cast<const uint64_t&>(handle);
}

/// View an integer as a path handle
inline path_handle_t& as_path_handle(uint64_t& value) {
    return reinterpret_cast<path_handle_t&>(value);
}

/// View a const integer as a const path handle
inline const path_handle_t& as_path_handle(const uint64_t& value) {
    return reinterpret_cast<const path_handle_t&>(value);
}

//
// Occurrence handles
//

/// View an occurrence handle as an integer
inline int64_t* as_integers(occurrence_handle_t& occurrence_handle) {
    return reinterpret_cast<int64_t*>(&occurrence_handle);
}

/// View a const occurrence handle as a const integer
inline const int64_t* as_integers(const occurrence_handle_t& occurrence_handle) {
    return reinterpret_cast<const int64_t*>(&occurrence_handle);
}



}


#endif
