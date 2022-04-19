/** \file mutable_path_metadata.cpp
 * Implement MutablePathMetadata interface's default implementation.
 */

#include "handlegraph/mutable_path_metadata.hpp"

#include <sstream>

namespace handlegraph {

path_handle_t MutablePathMetadata::create_path(const PathMetadata::Sense& sense,
                                               const std::string& sample,
                                               const std::string& locus,
                                               const int64_t& haplotype,
                                               const int64_t& phase_block,
                                               const std::pair<int64_t, int64_t>& subrange,
                                               bool is_circular) {
    
    return create_path_handle(PathMetadata::create_path_name(sense, sample, locus, haplotype, phase_block, subrange), is_circular);
}

}
