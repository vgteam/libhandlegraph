/** \file mutable_path_metadata.cpp
 * Implement MutablePathMetadata interface's default implementation.
 */

#include "handlegraph/mutable_path_metadata.hpp"

#include <sstream>

namespace handlegraph {

// And these are the constants for parsing path names out into metadata
const char MutablePathMetadata::SEPARATOR = '#';
const char MutablePathMetadata::RANGE_START_SEPARATOR = '[';
const char MutablePathMetadata::RANGE_END_SEPARATOR = '-';
const char MutablePathMetadata::RANGE_TERMINATOR = ']';

path_handle_t MutablePathMetadata::create_path(const PathMetadata::Sense& sense,
                                               const std::string& sample,
                                               const std::string& locus,
                                               const int64_t& haplotype,
                                               const int64_t& phase_block,
                                               const std::pair<int64_t, int64_t>& subrange,
                                               bool is_circular) {
                                               
    
    std::stringstream name_builder;
    
    if (sample != NO_SAMPLE_NAME) {
        if (sense == SENSE_GENERIC) {
            throw std::runtime_error("Generic path cannot be created with a sample");
        }
        name_builder << sample << SEPARATOR;
    }
    if (locus != NO_LOCUS_NAME) {
        name_builder << locus;
    } else {
        if (sense == SENSE_GENERIC) {
            throw std::runtime_error("Generic path cannot be created without a locus/name");
        } else if (sense == SENSE_REFERENCE) {
            throw std::runtime_error("Referecne path cannot be created without a locus");
        } else if (sense == SENSE_HAPLOTYPE) {
            throw std::runtime_error("Haplotype path cannot be created without a locus");
        }
    }
    if (haplotype != NO_HAPLOTYPE) {
        if (sense == SENSE_GENERIC) {
            throw std::runtime_error("Generic path cannot be created with a haplotype number");
        }
        name_builder << SEPARATOR << haplotype;
    } else {
        if (sense == SENSE_HAPLOTYPE) {
            throw std::runtime_error("Haplotype path cannot be created without a haplotype number");
        }
    }
    if (phase_block != NO_PHASE_BLOCK) {
        if (sense == SENSE_GENERIC) {
            throw std::runtime_error("Generic path cannot be created with a phase block");
        } else if (sense == SENSE_REFERENCE) {
            throw std::runtime_error("Reference path cannot be created with a phase block");
        }
        name_builder << SEPARATOR << phase_block;
    } else {
        if (sense == SENSE_HAPLOTYPE) {
            throw std::runtime_error("Haplotype path cannot be created without a phase block");
        }
    }
    if (subrange != NO_SUBRANGE) {
        // Everything can have a subrange.
        name_builder << RANGE_START_SEPARATOR << subrange.first;
        if (subrange.second != NO_END_POSITION) {
            name_builder << RANGE_END_SEPARATOR << subrange.second;
        }
        name_builder << RANGE_TERMINATOR;
    }
    
    return create_path_handle(name_builder.str(), is_circular);
}

}
