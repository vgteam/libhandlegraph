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
    for (auto& sense : {PathMetadata::SENSE_REFERENCE, PathMetadata::SENSE_GENERIC, PathMetadata::SENSE_HAPLOTYPE}) {
        // copy paths of that sense
        from->for_each_path_of_sense(sense, [&](const path_handle_t& path_handle) {
            copy_path(from, path_handle, into);
        });
    }
}

void copy_path(const PathHandleGraph* from, const path_handle_t& path,
               MutablePathHandleGraph* into) {
    
    // We know that the new path metadata API is there, so use it
    path_handle_t copied = into->create_path(from->get_sense(path),
                                             from->get_sample_name(path),
                                             from->get_locus_name(path),
                                             from->get_haplotype(path),
                                             from->get_phase_block(path),
                                             from->get_subrange(path),
                                             from->get_is_circular(path));
    
    // copy steps
    for (handle_t handle : from->scan_path(path)) {
        into->append_step(copied, into->get_handle(from->get_id(handle), from->get_is_reverse(handle)));
    }
}
}
}
