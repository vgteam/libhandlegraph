/** \file path_metadata.cpp
 * Implement PathMetadata interface's default implementation.
 */

#include "handlegraph/path_metadata.hpp"
#include <sstream>

namespace handlegraph {

// These are all our no-value placeholders.
const std::string PathMetadata::NO_SAMPLE_NAME = "";
const std::string PathMetadata::NO_LOCUS_NAME = "";
const size_t PathMetadata::NO_HAPLOTYPE = std::numeric_limits<size_t>::max();
const offset_t PathMetadata::NO_END_POSITION = std::numeric_limits<offset_t>::max();
const subrange_t PathMetadata::NO_SUBRANGE{PathMetadata::NO_END_POSITION, PathMetadata::NO_END_POSITION};

// Format examples:
// GRCh38#chrM (a reference)
// CHM13#chr12 (another reference)
// CHM13#chr12[300-400] (part of a reference)
// NA19239#1#chr1 (a diploid reference)
// NA29239#1#chr1#0 (a haplotype)
// 1[100] (part of a generic path)
// We don't support extraneous [] in name components in the structured format, or in names with ranges.
// TODO: escape them?

// So we match a regex for:
// One separator-free name component
// Up to 2 other optional separator-free name components, led by separators tacked on by non-capturing groups.
// Haplotype one must always be a number, or we aren't allowed to match (or we will crash trying to parse the number).
// Last one is lazy and allows colons but only takes as many characters as needed.
// Possibly a colon-delimited non-capturing group at the end
// Which has a number, and possibly a dash-led non-capturing group with a number.
// Match number:                         1           2          3             4        5
const std::regex PathMetadata::FORMAT(R"(([^[#]*)(?:#(\d+))?(?:#([^#]*?))?(?::(\d+)(?:-(\d+))?)?)");
// We also need to be able to parse scaffold names that we know can't contain subranges we respect.
// This uses the same groups, but the final peice is greedy
const std::regex PathMetadata::SCAFFOLD_FORMAT(R"(([^[#]*)(?:#(\d+))?(?:#([^#]*))?)");
const size_t PathMetadata::ASSEMBLY_OR_NAME_MATCH = 1;
const size_t PathMetadata::LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE = 2;
const size_t PathMetadata::HAPLOTYPE_MATCH = 2;
const size_t PathMetadata::LOCUS_MATCH_ANY = 3;
const size_t PathMetadata::RANGE_START_MATCH = 4;
const size_t PathMetadata::RANGE_END_MATCH = 5;

// And these are the constants for composing path names from metadata
const char PathMetadata::SEPARATOR = '#';
const char PathMetadata::RANGE_START_SEPARATOR = ':';
const char PathMetadata::RANGE_END_SEPARATOR = '-';


PathSense PathMetadata::get_sense(const path_handle_t& handle) const {
    return PathMetadata::parse_sense(get_path_name(handle));
}

std::string PathMetadata::get_sample_name(const path_handle_t& handle) const {
    return PathMetadata::parse_sample_name(get_path_name(handle));
}

std::string PathMetadata::get_locus_name(const path_handle_t& handle) const {
    return PathMetadata::parse_locus_name(get_path_name(handle));
}

size_t PathMetadata::get_haplotype(const path_handle_t& handle) const {
    return PathMetadata::parse_haplotype(get_path_name(handle));
}

subrange_t PathMetadata::get_subrange(const path_handle_t& handle) const {
    return PathMetadata::parse_subrange(get_path_name(handle));
}


std::string PathMetadata::get_path_scaffold_name(const path_handle_t& handle) const {
    // TODO: With the default implementations for the get methods this does a
    // lot of regex parsing just to split on the last colon (unless it doesn't
    // actually parse that way).
    // TODO: Write a consolidated get method?

    PathSense sense = get_sense(handle);
    std::string sample = get_sample_name(handle);
    std::string locus = get_locus_name(handle);
    size_t haplotype = get_haplotype(handle);
    
    // Just make a default style path name without a subrange.
    return create_path_name(sense, sample, locus, haplotype, NO_SUBRANGE);
}

region_t PathMetadata::get_path_region(const path_handle_t& handle) const {
    region_t region;
    region.first = get_path_scaffold_name(handle);
    region.second = get_subrange(handle);
    if (region.second == NO_SUBRANGE) {
        // We need to occupy the whole space
        region.second.first = 0;
        region.second.second = NO_END_POSITION;
    }
    if (region.second.second == NO_END_POSITION) {
        // Go ask for the path's length
        region.second.second = get_path_length(handle);
    }
    return region;
}

PathSense PathMetadata::parse_sense(const std::string& path_name) {
    // To get the sense we have to parse the whole thing and use its internal
    // guessing logic.
    PathSense sense;
    std::string sample;
    std::string locus;
    size_t haplotype;
    subrange_t subrange;
    parse_path_name(
        path_name,
        sense,
        sample,
        locus,
        haplotype,
        subrange
    );

    return sense;
}



std::string PathMetadata::parse_sample_name(const std::string& path_name) {
    // Match the regex
    std::smatch result;
    if (std::regex_match(path_name, result, FORMAT)) {
        if (result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].matched || result[LOCUS_MATCH_ANY].matched) {
            // There's a locus later, so the first thing doesn't have to be locus, so it can be sample.
            return result[ASSEMBLY_OR_NAME_MATCH].str();
        } else {
            // There's nothing but the locus and maybe a range.
            return NO_SAMPLE_NAME;
        }
    } else {
        // No sample name.
        return NO_SAMPLE_NAME;
    }
}


std::string PathMetadata::parse_locus_name(const std::string& path_name) {
    // Match the regex
    std::smatch result;
    if (std::regex_match(path_name, result, FORMAT)) {
        if (result[LOCUS_MATCH_ANY].matched) {
            // There's a locus and maybe a haplotype
            return result[LOCUS_MATCH_ANY].str();
        } else if (result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].matched) {
            // The locus wasn't pushed into its any slot, so we might have a
            // strictly numerical one here and no haplotype
            return result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].str();
        } else {
            // There's nothing but the locus and maybe a range.
            return result[ASSEMBLY_OR_NAME_MATCH].str();
        }
    } else {
        // Just the whole thing should come out here.
        return path_name;
    }
}


size_t PathMetadata::parse_haplotype(const std::string& path_name) {
    // Match the regex
    std::smatch result;
    if (std::regex_match(path_name, result, FORMAT)) {
        if (result[LOCUS_MATCH_ANY].matched && result[HAPLOTYPE_MATCH].matched) {
            // There's a locus and a haplotype, and we know the haplotype is a number.
            return std::stoll(result[HAPLOTYPE_MATCH].str());
        } else {
            // No haplotype is stored; if LOCUS_MATCH_ANY is empty and haplotype appears full, it's really the locus.
            return NO_HAPLOTYPE;
        }
    } else {
        // We can't parse this at all.
        return NO_HAPLOTYPE;
    }
}


subrange_t PathMetadata::parse_subrange(const std::string& path_name) {
    auto to_return = NO_SUBRANGE;
    
    // Match the regex
    std::smatch result;
    if (std::regex_match(path_name, result, FORMAT)) {
        if (result[RANGE_START_MATCH].matched) {
            // There is a range start, so pasre it
            to_return.first = std::stoll(result[RANGE_START_MATCH].str());
            if (result[RANGE_END_MATCH].matched) {
                // There is also an end, so parse that too
                to_return.second = std::stoll(result[RANGE_END_MATCH].str());
            }
        }
    }
    
    return to_return;
}

void PathMetadata::parse_path_name(const std::string& path_name,
                                   PathSense& sense,
                                   std::string& sample,
                                   std::string& locus,
                                   size_t& haplotype,
                                   subrange_t& subrange) {

    std::smatch result;
    auto matched = std::regex_match(path_name, result, FORMAT);
    
    // Parse out each piece.
    // TODO: can we unify this with the other places we parse out from the
    // regex? With yet a third set of functions?
    if (matched) {
        if (result[LOCUS_MATCH_ANY].matched && result[HAPLOTYPE_MATCH].matched) {
            // There's a haplotype and a locus and a sample
            sample = result[ASSEMBLY_OR_NAME_MATCH].str();
            locus = result[LOCUS_MATCH_ANY].str();
            haplotype = std::stoll(result[HAPLOTYPE_MATCH].str());
        } else if (result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].matched) {
            // There's a numerical locus but no haplotype, and a sample
            sample = result[ASSEMBLY_OR_NAME_MATCH].str();
            locus = result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].str();
            haplotype = NO_HAPLOTYPE;
        } else if (result[LOCUS_MATCH_ANY].matched) {
            // There's a non-numerical locus but no haplotype, and a sample
            sample = result[ASSEMBLY_OR_NAME_MATCH].str();
            locus = result[LOCUS_MATCH_ANY].str();
            haplotype = NO_HAPLOTYPE;
        } else {
            // There's nothing but the locus and maybe a range.
            sample = NO_SAMPLE_NAME;
            locus = result[ASSEMBLY_OR_NAME_MATCH].str();
            haplotype = NO_HAPLOTYPE;
        }
        
        if (result[RANGE_START_MATCH].matched) {
            // There is a range start, so parse it
            subrange.first = std::stoll(result[RANGE_START_MATCH].str());
            // Make sure to convert it to 0-based, end-exclusive coordinates.
            if (subrange.first == 0) {
                throw std::invalid_argument("Expected 1-based indexing in " + path_name);
            }
            subrange.first--;
            if (result[RANGE_END_MATCH].matched) {
                // There is also an end, so parse that too
                subrange.second = std::stoll(result[RANGE_END_MATCH].str());
            } else {
                subrange.second = NO_END_POSITION;
            }
        } else {
            subrange = NO_SUBRANGE;
        }

        if (result[LOCUS_MATCH_ANY].matched || result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].matched) {
            // It's a reference or haplotype because it has a locus and a sample.

            // TODO: We don't actually have a way to distinguish the sense by
            // name anymore without phase blocks. Cheat and abuse the fact that
            // references usually use haplotype 0 and haplotypes usually use 1
            // and 2.
            if (haplotype == 0 || haplotype == NO_HAPLOTYPE) {
                sense = PathSense::REFERENCE;
            } else {
                sense = PathSense::HAPLOTYPE;
            }
        } else {
            // It's just a one-piece generic name
            sense = PathSense::GENERIC;
        }
    } else {
        // Just a generic path where the locus is all of it.
        sense = PathSense::GENERIC;
        sample = NO_SAMPLE_NAME;
        locus = path_name;
        haplotype = NO_HAPLOTYPE;
        subrange = NO_SUBRANGE;
    }
}

void PathMetadata::parse_scaffold_name(const std::string& scaffold_name,
                                       std::string& sample,
                                       std::string& locus,
                                       size_t& haplotype) {

    std::smatch result;
    auto matched = std::regex_match(scaffold_name, result, SCAFFOLD_FORMAT);
    
    // Parse out each piece.
    if (matched) {
        if (result[LOCUS_MATCH_ANY].matched && result[HAPLOTYPE_MATCH].matched) {
            // There's a haplotype and a locus and a sample
            sample = result[ASSEMBLY_OR_NAME_MATCH].str();
            locus = result[LOCUS_MATCH_ANY].str();
            haplotype = std::stoll(result[HAPLOTYPE_MATCH].str());
        } else if (result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].matched) {
            // There's a numerical locus but no haplotype, and a sample
            sample = result[ASSEMBLY_OR_NAME_MATCH].str();
            locus = result[LOCUS_MATCH_NUMERICAL_WITHOUT_HAPLOTYPE].str();
            haplotype = NO_HAPLOTYPE;
        } else if (result[LOCUS_MATCH_ANY].matched) {
            // There's a non-numerical locus but no haplotype, and a sample
            sample = result[ASSEMBLY_OR_NAME_MATCH].str();
            locus = result[LOCUS_MATCH_ANY].str();
            haplotype = NO_HAPLOTYPE;
        } else {
            // There's nothing but the locus.
            sample = NO_SAMPLE_NAME;
            locus = result[ASSEMBLY_OR_NAME_MATCH].str();
            haplotype = NO_HAPLOTYPE;
        }
    } else {
        // Just a generic path where the locus is all of it.
        sample = NO_SAMPLE_NAME;
        locus = scaffold_name;
        haplotype = NO_HAPLOTYPE;
    }
}

std::string PathMetadata::create_path_name(const PathSense& sense,
                                           const std::string& sample,
                                           const std::string& locus,
                                           const size_t& haplotype,
                                           const subrange_t& subrange) {
    
    std::stringstream name_builder;
    
    if (sample != NO_SAMPLE_NAME) {
        if (sense == PathSense::GENERIC) {
            throw std::runtime_error("Generic path cannot have a sample");
        }
        name_builder << sample << SEPARATOR;
    } else {
        if (sense == PathSense::REFERENCE) {
            throw std::runtime_error("Reference path must have a sample name");
        } else if (sense == PathSense::HAPLOTYPE) {
            throw std::runtime_error("Haplotype path must have a sample name");
        }
    }
    if (haplotype != NO_HAPLOTYPE) {
        if (sense == PathSense::GENERIC) {
            throw std::runtime_error("Generic path cannot have a haplotype number");
        }
        name_builder << haplotype << SEPARATOR;
    } else {
        if (sense == PathSense::HAPLOTYPE) {
            throw std::runtime_error("Haplotype path must have a haplotype number");
        }
    }
    if (locus != NO_LOCUS_NAME) {
        name_builder << locus;
    } else {
        if (sense == PathSense::GENERIC) {
            throw std::runtime_error("Generic path must have a locus/name");
        } else if (sense == PathSense::REFERENCE) {
            throw std::runtime_error("Reference path must have a locus");
        } else if (sense == PathSense::HAPLOTYPE) {
            throw std::runtime_error("Haplotype path must have a locus");
        }
    }
    if (subrange != NO_SUBRANGE) {
        // Everything can have a subrange.
        // Make sure to convert to 1-based, end-inclusive coordinates.
        name_builder << RANGE_START_SEPARATOR << subrange.first + 1;
        if (subrange.second != NO_END_POSITION) {
            name_builder << RANGE_END_SEPARATOR << subrange.second;
        }
    }
    
    return name_builder.str();
}

bool PathMetadata::for_each_path_matching_impl(const std::unordered_set<PathSense>* senses,
                                               const std::unordered_set<std::string>* samples,
                                               const std::unordered_set<std::string>* loci,
                                               const std::unordered_set<size_t>* haplotypes,
                                               const std::function<bool(const path_handle_t&)>& iteratee) const {
    return for_each_path_handle_impl([&](const path_handle_t& handle) {
        if (senses && !senses->count(get_sense(handle))) {
            // Wrong sense
            return true;
        }
        if (samples && !samples->count(get_sample_name(handle))) {
            // Wrong sample
            return true;
        }
        if (loci && !loci->count(get_locus_name(handle))) {
            // Wrong sample
            return true;
        }
        if (haplotypes && !haplotypes->count(get_haplotype(handle))) {
            // Wrong haplotype
            return true;
        }
        // And emit any matching handles
        return iteratee(handle);
    });
}

bool PathMetadata::for_each_path_on_scaffold_impl(const std::string& scaffold_name, const std::function<bool(const path_handle_t&)>& iteratee) const {
    // Parse out the region into some structured metadata
    std::string sample;
    std::string locus;
    size_t haplotype;
    parse_scaffold_name(scaffold_name, sample, locus, haplotype);

    // Query the matching paths
    return for_each_path_matching({}, {sample}, {locus}, {haplotype}, iteratee);
}

bool PathMetadata::for_each_step_of_sense_impl(const handle_t& visited, const PathSense& sense, const std::function<bool(const step_handle_t&)>& iteratee) const {
    return for_each_step_on_handle_impl(visited, [&](const step_handle_t& handle) {
        if (get_sense(get_path_handle_of_step(handle)) != sense) {
            // Skip this non-matching path's step
            return true;
        }
        // And emit any steps on matching paths
        return iteratee(handle);
    });
}

}

