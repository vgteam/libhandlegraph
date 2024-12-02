#include "handlegraph/types.hpp"

#include "handlegraph/util.hpp"
#include "handlegraph/path_metadata.hpp"

#include <stdexcept>
#include <cstdlib>
#include <sstream>

/** \file types.cpp
 * Implement operators for libhandlegraph value types
 */

namespace handlegraph {

region_t parse_region(const std::string& region_text) {
    region_t result;
    size_t last_colon = region_text.rfind(":");
    if (last_colon == std::string::npos) {
        throw std::invalid_argument("Cannot parse coordinate region: No ':' found in " + region_text);
    }
    if (last_colon == 0) {
        throw std::invalid_argument("Cannot parse coordinate region: No text before ':' in " + region_text);
    }
    // Sequence name is everything up to the colon
    result.first = region_text.substr(0, last_colon);
    size_t dash = region_text.find("-", last_colon);
    if (dash == std::string::npos) {
        throw std::invalid_argument("Cannot parse coordinate region: No '-' found after last ':' in " + region_text);
    }
    
    // Make sure we actually have content on both sides of the dash
    if (dash == last_colon + 1) {
        throw std::invalid_argument("Cannot parse coordinate region: No text between last ':' and '-' in " + region_text);
    }

    if (region_text.size() == dash + 1) {
        throw std::invalid_argument("Cannot parse coordinate region: No text after '-' in " + region_text);
    }

    const char* start = region_text.c_str() + last_colon + 1;
    char* end;
    result.second.first = strtoll(start, &end, 10);
    if (end != region_text.c_str() + dash) {
        // We didn't parse everything before the dash
        throw std::invalid_argument("Cannot parse coordinate region: Non-number found before '-' in " + region_text);
    }
    if (result.second.first == 0) {
        throw std::invalid_argument("Cannot parse coordinate region: Expected 1-based indexing in " + region_text);
    }

    start = region_text.c_str() + dash + 1;
    result.second.second = strtoll(start, &end, 10);
    if (end != region_text.c_str() + region_text.size()) {
        // We didn't parse everything after the dash
        throw std::invalid_argument("Cannot parse coordinate region: Non-number found after '-' in " + region_text);
    }

    // Convert the range from 1-based, end-inclusive to 0-based, end-exclusive
    result.second.first -= 1;

    return result;
}

std::string to_string(const region_t& region) {
    std::stringstream ss;
    ss << region;
    return ss.str();
}

std::ostream& operator<<(std::ostream& out, const region_t region) {
    // Regions are always supposed to actually have start and end positions.
    if (region.second == PathMetadata::NO_SUBRANGE) {
        throw std::invalid_argument("Region on " + region.first + " does not have a subrange");
    }
    if (region.second.second == PathMetadata::NO_END_POSITION) {
        throw std::invalid_argument("Region on " + region.first + " starting at " + std::to_string(region.second.first) + " does not have an end position");
    }
    return out << region.first << ":" << (region.second.first + 1) << "-" << region.second.second;
}

bool operator==(const handle_t& a, const handle_t& b) {
    return as_integer(a) == as_integer(b);
}

bool operator!=(const handle_t& a, const handle_t& b) {
    return as_integer(a) != as_integer(b);
}

bool operator==(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) == as_integer(b);
}

bool operator!=(const path_handle_t& a, const path_handle_t& b) {
    return as_integer(a) != as_integer(b);
}

bool operator==(const step_handle_t& a, const step_handle_t& b) {
    return as_integers(a)[0] == as_integers(b)[0] && as_integers(a)[1] == as_integers(b)[1];
}

bool operator!=(const step_handle_t& a, const step_handle_t& b) {
    return !(a == b);
}

bool operator==(const net_handle_t& a, const net_handle_t& b) {
    return as_integer(a) == as_integer(b);
}

bool operator!=(const net_handle_t& a, const net_handle_t& b) {
    return !(a == b);
}

}


