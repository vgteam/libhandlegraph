#include "handlegraph/algorithms/split_strands.hpp"

#include <unordered_set>

namespace handlegraph {
namespace algorithms {

using namespace std;

unordered_map<handle_t, handle_t> split_strands(const HandleGraph* source, MutableHandleGraph* into) {
    
    if (into->get_node_count()) {
        cerr << "error:[algorithms] attempted to create strand-splitted graph in a non-empty graph" << endl;
        exit(1);
    }
    
    size_t source_nodes = source->get_node_count();
    
    unordered_map<handle_t, handle_t> node_translation;
    node_translation.reserve(2 * source_nodes);
    
    unordered_map<handle_t, handle_t> forward_node;
    forward_node.reserve(source_nodes);
    unordered_map<handle_t, handle_t> reverse_node;
    reverse_node.reserve(source_nodes);
    
    unordered_set<edge_t> edges;
    edges.reserve(3 * source_nodes); // Assumes 1.5 edges/node.
    
    source->for_each_handle([&](const handle_t& handle) {
        // create and record forward and reverse versions of each node
        handle_t fwd_handle = into->create_handle(source->get_sequence(handle));
        handle_t rev_handle = into->create_handle(source->get_sequence(source->flip(handle)));
        
        forward_node[handle] = fwd_handle;
        reverse_node[handle] = rev_handle;
        
        node_translation[fwd_handle] = handle;
        node_translation[rev_handle] = source->flip(handle);
        
        // collect all the edges
        source->follow_edges(handle, true, [&](const handle_t& prev) {
            edges.insert(source->edge_handle(prev, handle));
        });
        source->follow_edges(handle, false, [&](const handle_t& next) {
            edges.insert(source->edge_handle(handle, next));
        });
    });
    
    // translate each edge into two edges between forward-oriented nodes
    for (const edge_t& edge : edges) {
        handle_t fwd_prev = source->get_is_reverse(edge.first) ? reverse_node[source->flip(edge.first)]
        : forward_node[edge.first];
        handle_t fwd_next = source->get_is_reverse(edge.second) ? reverse_node[source->flip(edge.second)]
        : forward_node[edge.second];
        
        handle_t rev_prev = source->get_is_reverse(edge.second) ? forward_node[source->flip(edge.second)]
        : reverse_node[edge.second];
        handle_t rev_next = source->get_is_reverse(edge.first) ? forward_node[source->flip(edge.first)]
        : reverse_node[edge.first];
        
        into->create_edge(fwd_prev, fwd_next);
        into->create_edge(rev_prev, rev_next);
    }
    
    return node_translation;
}
}
}
