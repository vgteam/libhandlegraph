/**
 * \file canonical_gfa.cpp
 *
 * Implements an algorithm to writing a HandleGraph to an output stream in a
 * canonical GFA format. This can be used for computing stable graph names
 * using an appropriate hashing function (see https://github.com/jltsiren/pggname).
 */

#include "handlegraph/algorithms/canonical_gfa.hpp"

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

namespace handlegraph {
namespace algorithms {

//-----------------------------------------------------------------------------

template<typename T>
T node_id_as(const HandleGraph& graph, const handle_t& handle) = delete;

template<>
nid_t node_id_as<nid_t>(const HandleGraph& graph, const handle_t& handle) {
    return graph.get_id(handle);
}

template<>
std::string node_id_as<std::string>(const HandleGraph& graph, const handle_t& handle) {
    nid_t id = graph.get_id(handle);
    return std::to_string(id);
}

template<typename T>
std::vector<std::pair<T, handle_t>> nodes_in_order(const HandleGraph& graph) {
    std::vector<std::pair<T, handle_t>> id_to_handle;
    graph.for_each_handle([&](const handle_t& handle) {
        id_to_handle.emplace_back(node_id_as<T>(graph, handle), handle);
    });
    std::sort(id_to_handle.begin(), id_to_handle.end(), [](const std::pair<T, handle_t>& a, const std::pair<T, handle_t>& b) {
        return a.first < b.first;
    });
    return id_to_handle;
}

// Return value is (to_id, to_is_reverse, from_is_reverse) to make the tuple smaller.
// We need to sort the forward and reverse edges separately to get them sorted by
// (from_is_reverse, to_id, to_is_reverse).
template<typename T>
std::vector<std::tuple<T, bool, bool>> canonical_edges_in_order(const HandleGraph& graph, const T& from_id, handle_t forward_handle) {
    std::vector<std::tuple<T, bool, bool>> edges;

    // Canonical edges from the forward orientation.
    graph.follow_edges(forward_handle, false, [&](const handle_t& to_handle) {
        T to_id = node_id_as<T>(graph, to_handle);
        bool to_is_rev = graph.get_is_reverse(to_handle);
        if (from_id <= to_id) {
            // Self-loops from forward orientation are always canonical.
            edges.emplace_back(to_id, to_is_rev, false);
        }
    });
    std::sort(edges.begin(), edges.end());
    size_t forward_edges = edges.size();

    // Canonical edges from the reverse orientation.
    handle_t reverse_handle = graph.flip(forward_handle);
    graph.follow_edges(reverse_handle, false, [&](const handle_t& to_handle) {
        T to_id = node_id_as<T>(graph, to_handle);
        bool to_is_rev = graph.get_is_reverse(to_handle);
        if (from_id < to_id || (from_id == to_id && !to_is_rev)) {
            // Self-loops from reverse orientation are canonical only if
            // they go to the forward orientation.
            edges.emplace_back(to_id, to_is_rev, true);
        }
    });
    std::sort(edges.begin() + forward_edges, edges.end());

    return edges;
}

void write_gfa_s_line(const std::string& id, const std::string& sequence, std::ostream& out) {
    out.put('S'); out.put('\t'); out.write(id.data(), id.size());
    out.put('\t'); out.write(sequence.data(), sequence.size()); out.put('\n');
}

void write_gfa_l_line(const std::string& from_id, bool from_is_rev, const std::string& to_id, bool to_is_rev, std::ostream& out) {
    out.put('L'); out.put('\t'); out.write(from_id.data(), from_id.size());
    out.put('\t'); out.put(from_is_rev ? '-' : '+'); out.put('\t');
    out.write(to_id.data(), to_id.size());
    out.put('\t'); out.put(to_is_rev ? '-' : '+'); out.put('\n');
}

void canonical_gfa(const HandleGraph& graph, std::ostream& out, bool integer_ids) {
    if (integer_ids) {
        auto nodes = nodes_in_order<nid_t>(graph);
        for (const auto& node : nodes) {
            std::string from_id_str = std::to_string(node.first);
            std::string node_seq = graph.get_sequence(node.second); // TODO: GBWTGraph could avoid this copy.
            write_gfa_s_line(from_id_str, node_seq, out);
            auto edges = canonical_edges_in_order(graph, node.first, node.second);
            for (const auto& edge : edges) {
                write_gfa_l_line(from_id_str, std::get<2>(edge), std::to_string(std::get<0>(edge)), std::get<1>(edge), out);
            }
        }
    } else {
        auto nodes = nodes_in_order<std::string>(graph);
        for (const auto& node : nodes) {
            write_gfa_s_line(node.first, graph.get_sequence(node.second), out);
            auto edges = canonical_edges_in_order(graph, node.first, node.second);
            for (const auto& edge : edges) {
                write_gfa_l_line(node.first, std::get<2>(edge), std::get<0>(edge), std::get<1>(edge), out);
            }
        }
    }
    out.flush();
}

//-----------------------------------------------------------------------------

} // namespace algorithms
} // namespace handlegraph
