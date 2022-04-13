/** \file mutable_path_metadata.cpp
 * Implement MutablePathMetadata interface's default implementation.
 */

#include "handlegraph/mutable_path_metadata.hpp"

#include <sstream>

namespace handlegraph {

path_handle_t MutablePathMetadata::create_path(const PathSense& sense,
                                               const std::string& sample,
                                               const std::string& locus,
                                               const int64_t& haplotype,
                                               const int64_t& phase_block,
                                               const subrange_t& subrange,
                                               bool is_circular) {
    
    return create_path_handle(PathMetadata::create_path_name(sense, sample, locus, haplotype, phase_block, subrange), is_circular);
}

}
