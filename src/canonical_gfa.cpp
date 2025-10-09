/**
 * \file canonical_gfa.cpp
 *
 * Implements an algorithm to writing a HandleGraph to an output stream in a
 * canonical GFA format. This can be used for computing stable graph names
 * using an appropriate hashing function (see https://github.com/jltsiren/pggname).
 */

#include "handlegraph/algorithms/canonical_gfa.hpp"

#include <algorithm>
#include <map>
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
std::map<T, handle_t> nodes_in_order(const HandleGraph& graph) {
    std::map<T, handle_t> id_to_handle;
    graph.for_each_handle([&](const handle_t& handle) {
        id_to_handle[node_id_as<T>(graph, handle)] = handle;
    });
    return id_to_handle;
}

template<typename T>
std::vector<std::tuple<bool, T, bool>> canonical_edges_in_order(const HandleGraph& graph, const T& from_id, handle_t forward_handle) {
    std::vector<std::tuple<bool, T, bool>> edges;

    // Canonical edges from the forward orientation.
    graph.follow_edges(forward_handle, false, [&](const handle_t& to_handle) {
        T to_id = node_id_as<T>(graph, to_handle);
        bool to_is_rev = graph.get_is_reverse(to_handle);
        if (from_id <= to_id) {
            // Self-loops from forward orientation are always canonical.
            edges.emplace_back(false, to_id, to_is_rev);
        }
    });

    // Canonical edges from the reverse orientation.
    handle_t reverse_handle = graph.flip(forward_handle);
    graph.follow_edges(reverse_handle, false, [&](const handle_t& to_handle) {
        T to_id = node_id_as<T>(graph, to_handle);
        bool to_is_rev = graph.get_is_reverse(to_handle);
        if (from_id < to_id || (from_id == to_id && !to_is_rev)) {
            // Self-loops from reverse orientation are canonical only if
            // they go to the forward orientation.
            edges.emplace_back(true, to_id, to_is_rev);
        }
    });

    std::sort(edges.begin(), edges.end());
    edges.resize(std::unique(edges.begin(), edges.end()) - edges.begin());

    return edges;
}

void canonical_gfa(const HandleGraph& graph, std::ostream& out, bool integer_ids) {
    if (integer_ids) {
        auto nodes = nodes_in_order<nid_t>(graph);
        for (const auto& node : nodes) {
            out << "S\t" << node.first << "\t" << graph.get_sequence(node.second) << "\n";
            auto edges = canonical_edges_in_order(graph, node.first, node.second);
            for (const auto& edge : edges) {
                out << "L\t" << node.first << "\t" << (std::get<0>(edge) ? "-" : "+") << "\t"
                    << std::get<1>(edge) << "\t" << (std::get<2>(edge) ? "-" : "+") << "\n";
            }
        }
    } else {
        auto nodes = nodes_in_order<std::string>(graph);
        for (const auto& node : nodes) {
            out << "S\t" << node.first << "\t" << graph.get_sequence(node.second) << "\n";
            auto edges = canonical_edges_in_order(graph, node.first, node.second);
            for (const auto& edge : edges) {
                out << "L\t" << node.first << "\t" << (std::get<0>(edge) ? "-" : "+") << "\t"
                    << std::get<1>(edge) << "\t" << (std::get<2>(edge) ? "-" : "+") << "\n";
            }
        }
    }
}

//-----------------------------------------------------------------------------

} // namespace algorithms
} // namespace handlegraph
