#ifndef HANDLEGRAPH_MUTABLE_PATH_METADATA_HPP_INCLUDED
#define HANDLEGRAPH_MUTABLE_PATH_METADATA_HPP_INCLUDED

/** \file 
 * Defines the mutable metadata API for paths
 */

#include "handlegraph/path_metadata.hpp"

#include <vector>

namespace handlegraph {

/**
 * This is the interface for mutable embedded path and haplotype thread
 * metadata (see PathMetadata).
 *
 * Comes with a default implementation of this interface, based on
 * a name-based create_path_handle() and special path name formatting.
 *
 */
class MutablePathMetadata {
public:
    
    virtual ~MutablePathMetadata() = default;
    
    ////////////////////////////////////////////////////////////////////////////
    // Mutable path metadata interface that needs to be implemented
    ////////////////////////////////////////////////////////////////////////////
    
    /**
     * Add a path with the given metadata. Any item can be the
     *  corresponding unset sentinel (PathMetadata::NO_LOCUS_NAME, PathMetadata::NO_PHASE_BLOCK, etc.).
     * 
     *  Implementations may refuse to store paths-or-threads of certain senses
     *  when relevant fields are unset.
     * 
     * Handles to other paths must
     * remain valid.
     */
    virtual path_handle_t create_path(const PathMetadata::Sense& sense,
                                      const std::string& sample,
                                      const std::string& locus,
                                      const int64_t& haplotype,
                                      const int64_t& phase_block,
                                      const std::pair<int64_t, int64_t>& subrange,
                                      bool is_circular = false);

protected:
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing methods for default implementation
    ////////////////////////////////////////////////////////////////////////////

    /**
     * Create a path with the given name. The caller must ensure that no path
     * with the given name exists already, or the behavior is undefined.
     * Returns a handle to the created empty path. Handles to other paths must
     * remain valid.
     */
    virtual path_handle_t create_path_handle(const std::string& name,
                                             bool is_circular = false) = 0;
    
    
};

}

#endif

