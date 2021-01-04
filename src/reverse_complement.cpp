#include "handlegraph/algorithms/reverse_complement.hpp"

#include <unordered_map>

namespace handlegraph {
namespace algorithms {

using namespace std;

void reverse_complement_graph(const HandleGraph* source, MutableHandleGraph* into) {
    
    if (into->get_node_count()) {
        cerr << "error:[algorithms] attempted to create reversed graph in a non-empty graph" << endl;
        exit(1);
    }
    
    // make the nodes in reverse orientation
    source->for_each_handle([&](const handle_t& handle) {
        into->create_handle(source->get_sequence(source->flip(handle)),
                            source->get_id(handle));
    });
    
    // make the edges
    source->for_each_edge([&](const edge_t& edge) {
        // actually make the edge
        into->create_edge(into->get_handle(source->get_id(edge.second),
                                           source->get_is_reverse(edge.second)),
                          into->get_handle(source->get_id(edge.first),
                                           source->get_is_reverse(edge.first)));
        
        // always keep going
        return true;
    });
}
}
}
