#include "handlegraph/algorithms/copy_graph.hpp"

#include <stdexcept>

namespace handlegraph {
namespace algorithms {

void copy_handle_graph(const HandleGraph* from, MutableHandleGraph* into) {
    
    
    if (from == nullptr) {
        throw std::runtime_error("error:[copy_handle_graph] must supply graph to copy from");
    }
    if (into == nullptr) {
        throw std::runtime_error("error:[copy_handle_graph] must supply graph to copy into");
    }
    
    // TODO: some code paths depend on this algorithm for appending one graph onto another
    //        if (into->get_node_count() > 0) {
    //            throw runtime_error("error:[copy_handle_graph] cannot copy into a non-empty graph");
    //        }
    
    // copy nodes
    from->for_each_handle([&](const handle_t& handle) {
        into->create_handle(from->get_sequence(handle), from->get_id(handle));
    });
    
    // copy edges
    from->for_each_edge([&](const edge_t& edge_handle) {
        into->create_edge(into->get_handle(from->get_id(edge_handle.first),
                                           from->get_is_reverse(edge_handle.first)),
                          into->get_handle(from->get_id(edge_handle.second),
                                           from->get_is_reverse(edge_handle.second)));
    });
}

void copy_path_handle_graph(const PathHandleGraph* from, MutablePathMutableHandleGraph* into) {
    
    // copy topology
    copy_handle_graph(from, into);
    
    // TODO: some code paths depend on this algorithm for appending one graph onto another
    //        if (into->get_path_count() > 0) {
    //            throw runtime_error("error:[copy_handle_graph] cannot copy into a non-empty graph");
    //        }
    
    // For every sense of path
    for (auto& sense : {PathSense::REFERENCE, PathSense::GENERIC, PathSense::HAPLOTYPE}) {
        // copy paths of that sense
        from->for_each_path_of_sense(sense, [&](const path_handle_t& path_handle) {
            copy_path(from, path_handle, into);
        });
    }
}

void copy_path(const PathHandleGraph* from, const path_handle_t& from_path,
               MutablePathHandleGraph* into) {
    
    // Make a new path with the same metadata as the old path
    path_handle_t into_path = into->create_path(from->get_sense(from_path),
                                                from->get_sample_name(from_path),
                                                from->get_locus_name(from_path),
                                                from->get_haplotype(from_path),
                                                from->get_phase_block(from_path),
                                                from->get_subrange(from_path),
                                                from->get_is_circular(from_path));
    
    // Copy all the steps over
    copy_path(from, from_path, into, into_path);
}

void copy_path(const PathHandleGraph* from, const path_handle_t& from_path,
               MutablePathHandleGraph* into, const path_handle_t& into_path) {
    
    // copy steps
    for (handle_t handle : from->scan_path(from_path)) {
        into->append_step(into_path, into->get_handle(from->get_id(handle), from->get_is_reverse(handle)));
    }
}

}
}
