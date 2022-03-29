#ifndef HANDLEGRAPH_PATH_METADATA_HPP_INCLUDED
#define HANDLEGRAPH_PATH_METADATA_HPP_INCLUDED

/** \file 
 * Defines the metadata API for paths
 */

#include "handlegraph/handle_graph.hpp"

#include <utility>
#include <string>
#include <regex>

namespace handlegraph {

/**
 * This is the interface for embedded path and haplotype thread metadata.
 *
 * Comes with a default implementation of this interface, based on
 * a get_path_name() and special path name formatting.
 *
 * Our model is that paths come in different "senses":
 *
 * - SENSE_GENERIC: a generic named path. Has a "locus" name.
 *
 * - SENSE_REFERENCE: a part of a reference assembly. Has a "sample" name, a
 *   "locus" name, and a haplotype number.
 *
 * - SENSE_HAPLOTYPE: a haplotype from a particular individual. Has a "sample"
 *   name, a "locus" name, a haplotype number, and a phase block identifier.
 *
 * Paths of all sneses can represent subpaths, with bounds.
 *
 * Depending on sense, a path might have:
 *
 * - Sample: sample or assembly name.
 *
 * - Locus: contig, scaffold, or gene name path either represents in its
 *   assembly or is an allele of in its sample.
 *
 * - Haplotype number: number identifying which haplotype of a locus is being
 *   represented. GFA uses a convention where the presence of a haplotype 0
 *   implies that only one haplotype is present.
 *
 * - Phase block identifier: Distinguishes fragments of a haplotype that are
 *   phased but not necessarily part of a single self-consistent scaffold (often
 *   due to self-contradictory VCF information). Must be unique within a sample,
 *   locus, and haplotype. May be a number or a start coordinate.
 *
 * - Bounds, for when a path as stored gives only a sub-range of a conceptually
 *   longer path. Multiple items can be stored with identical metadata in the
 *   other fields if their bounds are non-overlapping.
 * TODO: Interaction with phase block in GBWT???
 */
class PathMetadata {
public:
    
    virtual ~PathMetadata() = default;
    
    /// Each path always has just one sense.
    enum Sense {
        SENSE_GENERIC,
        SENSE_REFERENCE,
        SENSE_HAPLOTYPE
    };
    
    ////////////////////////////////////////////////////////////////////////////
    // Path metadata interface that has a default implementation
    ////////////////////////////////////////////////////////////////////////////
    
    /// What is the given path meant to be representing?
    virtual Sense get_sense(const path_handle_t& handle) const;
    
    /// Get the name of the sample or assembly asociated with the
    /// path-or-thread, or NO_SAMPLE_NAME if it does not belong to one.
    virtual std::string get_sample_name(const path_handle_t& handle) const;
    static const std::string NO_SAMPLE_NAME;
    
    /// Get the name of the contig or gene asociated with the path-or-thread,
    /// or NO_LOCUS_NAME if it does not belong to one.
    virtual std::string get_locus_name(const path_handle_t& handle) const;
    static const std::string NO_LOCUS_NAME;
    
    /// Get the haplotype number (0 or 1, for diploid) of the path-or-thread,
    /// or NO_HAPLOTYPE if it does not belong to one.
    virtual int64_t get_haplotype(const path_handle_t& handle) const;
    static const int64_t NO_HAPLOTYPE;
    
    /// Get the phase block number (contiguously phased region of a sample,
    /// contig, and haplotype) of the path-or-thread, or NO_PHASE_BLOCK if it
    /// does not belong to one.
    virtual int64_t get_phase_block(const path_handle_t& handle) const;
    static const int64_t NO_PHASE_BLOCK;
    
    /// Get the bounds of the path-or-thread that are actually represented
    /// here. Should be NO_SUBRANGE if the entirety is represented here, and
    /// 0-based inclusive start and exclusive end positions of the stored 
    /// region on the full path-or-thread if a subregion is stored.
    ///
    /// If no end position is stored, NO_END_POSITION may be returned for the
    /// end position.
    virtual std::pair<int64_t, int64_t> get_subrange(const path_handle_t& handle) const;
    static const std::pair<int64_t, int64_t> NO_SUBRANGE;
    static const int64_t NO_END_POSITION;
    
    ////////////////////////////////////////////////////////////////////////////
    // Tools for converting back and forth with single-string path names
    ////////////////////////////////////////////////////////////////////////////
    
    /// Extract the sense of a path from the given formatted path name, if
    /// possible. If not possible, return SENSE_GENERIC.
    static Sense parse_sense(const std::string& path_name);
    
    /// Get the name of the sample or assembly embedded in the given formatted
    /// path name, or NO_SAMPLE_NAME if it does not belong to one.
    static std::string parse_sample_name(const std::string& path_name);
    
    /// Get the name of the contig or gene  embedded in the given formatted
    /// path name, or NO_LOCUS_NAME if it does not belong to one.
    static std::string parse_locus_name(const std::string& path_name);
    
    /// Get the haplotype number (0 or 1, for diploid) embedded in the given
    /// formatted path name, or NO_HAPLOTYPE if it does not belong to one.
    static int64_t parse_haplotype(const std::string& path_name);
    
    /// Get the phase block number (contiguously phased region of a sample,
    /// contig, and haplotype) embedded in the given formatted path name, or
    /// NO_PHASE_BLOCK if it does not belong to one.
    static int64_t parse_phase_block(const std::string& path_name);
    
    /// Get the bounds embedded in the given formatted path name, or
    /// NO_SUBRANGE if they are absent. If no end position is stored,
    /// NO_END_POSITION may be returned for the end position.
    static std::pair<int64_t, int64_t> parse_subrange(const std::string& path_name);
    
    /// Decompose a formatted path name into metadata.
    static void parse_path_name(const std::string& path_name,
                                PathMetadata::Sense& sense,
                                std::string& sample,
                                std::string& locus,
                                int64_t& haplotype,
                                int64_t& phase_block,
                                std::pair<int64_t, int64_t>& subrange);

    /// Compose a formatted path name for the given metadata. Any item can be
    /// the corresponding unset sentinel (PathMetadata::NO_LOCUS_NAME,
    /// PathMetadata::NO_PHASE_BLOCK, etc.).
    static std::string create_path_name(const PathMetadata::Sense& sense,
                                        const std::string& sample,
                                        const std::string& locus,
                                        const int64_t& haplotype,
                                        const int64_t& phase_block,
                                        const std::pair<int64_t, int64_t>& subrange);
    
    ////////////////////////////////////////////////////////////////////////////
    // Stock interface that uses backing virtual methods
    ////////////////////////////////////////////////////////////////////////////
    
    /// Loop through all the paths with the given sense. Returns false and
    /// stops if the iteratee returns false.
    template<typename Iteratee>
    bool for_each_path_of_sense(const Sense& sense, const Iteratee& iteratee) const;
    
    /// Loop through all steps on the given handle for paths with the given
    /// sense. Returns false and stops if the iteratee returns false.
    /// TODO: Take the opportunity to make steps track orientation better?
    template<typename Iteratee>
    bool for_each_step_of_sense(const handle_t& visited, const Sense& sense, const Iteratee& iteratee) const;
    
protected:
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing protected virtual methods that have a default implementation
    ////////////////////////////////////////////////////////////////////////////
    
    // The default implementations for these scanning methods falls back on the
    // generic for_each_path_handle/for_each_step_on_handle and just filters.
    // If those are implemented to elide haplotype paths, these need to be
    // implemented to allow them to be retrieved.
    
    /// Loop through all the paths with the given sense. Returns false and
    /// stops if the iteratee returns false.
    virtual bool for_each_path_of_sense_impl(const Sense& sense, const std::function<bool(const path_handle_t&)>& iteratee) const;
    
    /// Loop through all steps on the given handle for paths with the given
    /// sense. Returns false and stops if the iteratee returns false.
    virtual bool for_each_step_of_sense_impl(const handle_t& visited, const Sense& sense, const std::function<bool(const step_handle_t&)>& iteratee) const;
    
    ////////////////////////////////////////////////////////////////////////////
    // Backing methods that need to be implemented for default implementation
    ////////////////////////////////////////////////////////////////////////////
    
    // These are really PathHandleGraph methods; we just have to know they exist
    
    /// Look up the name of a path from a handle to it
    virtual std::string get_path_name(const path_handle_t& path_handle) const = 0;
    
    /// Returns a handle to the path that an step is on
    virtual path_handle_t get_path_handle_of_step(const step_handle_t& step_handle) const = 0;
    
    /// Execute a function on each path in the graph. If it returns false, stop
    /// iteration. Returns true if we finished and false if we stopped early.
    ///
    /// If the graph contains compressed haplotype paths and properly
    /// implements for_each_path_of_sense to retrieve them, they should not be
    /// visible here. Only reference or generic named paths should be visible.
    virtual bool for_each_path_handle_impl(const std::function<bool(const path_handle_t&)>& iteratee) const = 0;
    
    /// Execute a function on each step of a handle in any path. If it
    /// returns false, stop iteration. Returns true if we finished and false if
    /// we stopped early.
    ///
    /// If the graph contains compressed haplotype paths and properly
    /// implements for_each_step_of_sense to find them, they should not be
    /// visible here. Only reference or generic named paths should be visible.
    virtual bool for_each_step_on_handle_impl(const handle_t& handle,
        const std::function<bool(const step_handle_t&)>& iteratee) const = 0;
    
private:
    
    ////////////////////////////////////////////////////////////////////////////
    // Internal machinery for path name mini-format
    ////////////////////////////////////////////////////////////////////////////
    
    static const std::regex FORMAT;
    static const size_t ASSEMBLY_OR_NAME_MATCH;
    static const size_t LOCUS_MATCH_WITHOUT_HAPLOTYPE;
    static const size_t HAPLOTYPE_MATCH;
    static const size_t LOCUS_MATCH_WITH_HAPLOTYPE;
    static const size_t PHASE_BLOCK_MATCH;
    static const size_t RANGE_START_MATCH;
    static const size_t RANGE_END_MATCH;
    
    /// Separator used to separate path name components
    static const char SEPARATOR;
    // Ranges are set off with some additional characters.
    static const char RANGE_START_SEPARATOR;
    static const char RANGE_END_SEPARATOR;
    static const char RANGE_TERMINATOR;
};

////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

template<typename Iteratee>
bool PathMetadata::for_each_path_of_sense(const Sense& sense, const Iteratee& iteratee) const {
    return for_each_path_of_sense_impl(sense, BoolReturningWrapper<Iteratee>::wrap(iteratee));
}

template<typename Iteratee>
bool PathMetadata::for_each_step_of_sense(const handle_t& visited, const Sense& sense, const Iteratee& iteratee) const {
    return for_each_step_of_sense_impl(visited, sense, BoolReturningWrapper<Iteratee>::wrap(iteratee));
}

}
#endif
