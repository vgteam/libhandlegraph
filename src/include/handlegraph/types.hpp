#ifndef HANDLEGRAPH_TYPES_HPP_INCLUDED
#define HANDLEGRAPH_TYPES_HPP_INCLUDED

/** \file
 * Defines the types used for handles, and user-facing operators to work with them.
 */

#include <cstdint>
#include <utility>
#include <functional>

namespace handlegraph {

/// Represents an id
typedef int64_t nid_t;
    
[[deprecated("id_t collides with a standard type, use nid_t instead")]]
typedef nid_t id_t;
    
/// Represents an offset
typedef std::size_t off_t;
    
/// Represents a position
typedef std::tuple<nid_t, bool, off_t> pos_t;
    
/// Represents the internal id of a node traversal
struct handle_t { char data[sizeof(nid_t)]; };
    
/// Represents an edge in terms of its endpoints
typedef std::pair<handle_t, handle_t> edge_t;
    
/// Represents the internal id of a path entity
struct path_handle_t { char data[sizeof(int64_t)]; };
    
/// A step handle is an opaque reference to a single step of an oriented node on a path in a graph
struct step_handle_t { char data[2 * sizeof(int64_t)]; };

/// Define equality on handles
bool operator==(const handle_t& a, const handle_t& b);

/// Define inequality on handles
bool operator!=(const handle_t& a, const handle_t& b);

/// Define equality on path handles
bool operator==(const path_handle_t& a, const path_handle_t& b);

/// Define inequality on path handles
bool operator!=(const path_handle_t& a, const path_handle_t& b);

/// Define equality on step handles
bool operator==(const step_handle_t& a, const step_handle_t& b);

/// Define inequality on step handles
bool operator!=(const step_handle_t& a, const step_handle_t& b);

}

// Hashes need to be in the std namespace
namespace std {

/**
 * Define hashes for handles.
 */
template<> struct hash<handlegraph::handle_t> {
public:
    inline size_t operator()(const handlegraph::handle_t& handle) const {
        // TODO: We can't include util.cpp for as_integer because it includes us!
        // But we need anyone with access to the handle_t to be able to hash it.
        // So we just convert to integer manually.
        return std::hash<int64_t>()(reinterpret_cast<const uint64_t&>(handle));
    }
};

/**
 * Define hashes for path handles.
 */
template<> struct hash<handlegraph::path_handle_t> {
public:
    inline size_t operator()(const handlegraph::path_handle_t& path_handle) const {
        return std::hash<int64_t>()(reinterpret_cast<const uint64_t&>(path_handle));
    }
};
    
/**
 * Define hashes for step handles.
 */
template<> struct hash<handlegraph::step_handle_t> {
public:
    inline size_t operator()(const handlegraph::step_handle_t& step_handle) const {
        size_t hsh1 = std::hash<int64_t>()(reinterpret_cast<const int64_t*>(&step_handle)[0]);
        size_t hsh2 = std::hash<int64_t>()(reinterpret_cast<const int64_t*>(&step_handle)[1]);
        // Boost combine for hash values
        return hsh1 ^ (hsh2 + 0x9e3779b9 + (hsh1<<6) + (hsh1>>2));
    }
};

}


#endif
