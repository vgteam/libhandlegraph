/**
 * \file dagify.cpp
 *
 * Implements an algorithm to convert a single-stranded graph into a DAG
 */

//#define debug_dagify
#include <vector>
#include <unordered_set>
#include <atomic>

#include "handlegraph/expanding_overlay_graph.hpp"
#include "handlegraph/algorithms/dagify.hpp"
#include "handlegraph/algorithms/is_single_stranded.hpp"
#include "handlegraph/algorithms/strongly_connected_components.hpp"
#include "handlegraph/algorithms/eades_algorithm.hpp"
#include "handlegraph/algorithms/internal/dfs.hpp"

namespace handlegraph {
namespace algorithms {

using namespace std;

// TODO: ugly copypasta

class SubHandleGraph : public ExpandingOverlayGraph {
public:
    
    /// Initialize as empty subgraph of a super graph
    SubHandleGraph(const HandleGraph* super);
    
    /// Add a node from the super graph to the subgraph. Must be a handle to the
    /// super graph. No effect if the node is already included in the subgraph.
    /// Generally invalidates the results of any previous algorithms.
    void add_handle(const handle_t& handle);
    
    //////////////////////////
    /// HandleGraph interface
    //////////////////////////
    
    // Method to check if a node exists by ID
    bool has_node(nid_t node_id) const;
    
    /// Look up the handle for the node with the given ID in the given orientation
    handle_t get_handle(const nid_t& node_id, bool is_reverse = false) const;
    
    /// Get the ID from a handle
    nid_t get_id(const handle_t& handle) const;
    
    /// Get the orientation of a handle
    bool get_is_reverse(const handle_t& handle) const;
    
    /// Invert the orientation of a handle (potentially without getting its ID)
    handle_t flip(const handle_t& handle) const;
    
    /// Get the length of a node
    size_t get_length(const handle_t& handle) const;
    
    /// Get the sequence of a node, presented in the handle's local forward
    /// orientation.
    string get_sequence(const handle_t& handle) const;
    
    /// Loop over all the handles to next/previous (right/left) nodes. Passes
    /// them to a callback which returns false to stop iterating and true to
    /// continue. Returns true if we finished and false if we stopped early.
    bool follow_edges_impl(const handle_t& handle, bool go_left, const function<bool(const handle_t&)>& iteratee) const;
    
    /// Loop over all the nodes in the graph in their local forward
    /// orientations, in their internal stored order. Stop if the iteratee
    /// returns false. Can be told to run in parallel, in which case stopping
    /// after a false return value is on a best-effort basis and iteration
    /// order is not defined.
    bool for_each_handle_impl(const function<bool(const handle_t&)>& iteratee, bool parallel = false) const;
    
    /// Return the number of nodes in the graph
    size_t get_node_count() const;
    
    /// Return the smallest ID in the graph, or some smaller number if the
    /// smallest ID is unavailable. Return value is unspecified if the graph is empty.
    nid_t min_node_id() const;
    
    /// Return the largest ID in the graph, or some larger number if the
    /// largest ID is unavailable. Return value is unspecified if the graph is empty.
    nid_t max_node_id() const;
    
    ///////////////////////////////////
    /// ExpandingOverlayGraph interface
    ///////////////////////////////////
    
    /**
     * Returns the handle in the underlying graph that corresponds to a handle in the
     * overlay
     */
    handle_t get_underlying_handle(const handle_t& handle) const;
    
private:
    const HandleGraph* super = nullptr;
    unordered_set<nid_t> contents;
    // keep track of these separately rather than use an ordered set
    nid_t min_id = numeric_limits<nid_t>::max();
    nid_t max_id = numeric_limits<nid_t>::min();
    
};



SubHandleGraph::SubHandleGraph(const HandleGraph* super) : super(super) {
    // nothing to do
}

void SubHandleGraph::add_handle(const handle_t& handle) {
    
    nid_t node_id = super->get_id(handle);
    
    min_id = min(node_id, min_id);
    max_id = max(node_id, max_id);
    
    contents.insert(node_id);
}

bool SubHandleGraph::has_node(nid_t node_id) const {
    return contents.count(node_id);
}

handle_t SubHandleGraph::get_handle(const nid_t& node_id, bool is_reverse) const {
    if (!contents.count(node_id)) {
        cerr << "error:[SubHandleGraph] subgraph does not contain node with ID " << node_id << endl;
        exit(1);
    }
    return super->get_handle(node_id, is_reverse);
}

nid_t SubHandleGraph::get_id(const handle_t& handle) const {
    return super->get_id(handle);
}

bool SubHandleGraph::get_is_reverse(const handle_t& handle) const {
    return super->get_is_reverse(handle);
}

handle_t SubHandleGraph::flip(const handle_t& handle) const {
    return super->flip(handle);
}

size_t SubHandleGraph::get_length(const handle_t& handle) const {
    return super->get_length(handle);
}

string SubHandleGraph::get_sequence(const handle_t& handle) const {
    return super->get_sequence(handle);
}

bool SubHandleGraph::follow_edges_impl(const handle_t& handle, bool go_left, const function<bool(const handle_t&)>& iteratee) const {
    // only let it travel along edges whose endpoints are in the subgraph
    bool keep_going = true;
    super->follow_edges(handle, go_left, [&](const handle_t& handle) {
        if (contents.count(super->get_id(handle))) {
            keep_going = iteratee(handle);
        }
        return keep_going;
    });
    return keep_going;
}

bool SubHandleGraph::for_each_handle_impl(const function<bool(const handle_t&)>& iteratee, bool parallel) const {
    // non-parallel so we don't pull in any dependencies
    for (nid_t node_id : contents) {
        if (!iteratee(super->get_handle(node_id))) {
            return false;
        }
    }
    return true;
}

size_t SubHandleGraph::get_node_count() const {
    return contents.size();
}

nid_t SubHandleGraph::min_node_id() const {
    return min_id;
}

nid_t SubHandleGraph::max_node_id() const {
    return max_id;
}

handle_t SubHandleGraph::get_underlying_handle(const handle_t& handle) const {
    return handle;
}


// TODO: SubHandleGraph port over

/// Internal implementation that exposes the ID translation back to the original graph and the injection into the dagified graph, with all copies in order. 
static std::pair<std::unordered_map<nid_t, nid_t>, unordered_map<handle_t, vector<handle_t>>> dagify_internal(const HandleGraph* graph,
                                                                                                              MutableHandleGraph* into,
                                                                                                              size_t min_preserved_path_length) {
    
    // initialize the translator from the dagified graph back to the original graph
    unordered_map<nid_t, nid_t> translator;
    
    // generate a canonical orientation across the graph
    vector<handle_t> orientation = single_stranded_orientation(graph);
    
    if (orientation.size() < graph->get_node_count()) {
        cerr << "error:[dagify] Dagify algorithm only valid on graphs with a single stranded orientation, consider using split_strands first" << endl;
        exit(1);
    }
    
#ifdef debug_dagify
    cerr << "got canonical orientation:" << endl;
    for (const handle_t& h : orientation) {
        cerr << "\t" << graph->get_id(h) << (graph->get_is_reverse(h) ? "-" : "+") << endl;
    }
#endif
    
    // mark the ones that whose canonical orientation is reversed
    unordered_set<nid_t> reversed_nodes;
    for (size_t i = 0; i < orientation.size(); i++) {
        if (graph->get_is_reverse(orientation[i])) {
            reversed_nodes.insert(graph->get_id(orientation[i]));
        }
    }
    
    // find the strongly connected components of the original graph
    vector<unordered_set<nid_t>> strong_components = strongly_connected_components(graph);
    
#ifdef debug_dagify
    cerr << "got strongly connected components:" << endl;
    for (size_t i = 0; i < strong_components.size(); i++) {
        cerr << "\tcomponent " << i << endl;
        for (nid_t nid : strong_components[i]) {
            cerr << "\t\t" << nid << endl;
        }
    }
#endif
    
    // duplicate strongly connected components into the dagified graph in such a way
    // that paths are preserved
    
    // a tracker for which SCC a node belongs to
    unordered_map<nid_t, size_t> component_of;
    // a map from a node in the original graph to all its copies (in order) in the
    // dagified graph
    unordered_map<handle_t, vector<handle_t>> injector;
    for (size_t i = 0; i < strong_components.size(); i++) {
        
#ifdef debug_dagify
        cerr << "handling component " << i << endl;
#endif
        
        // keep track of which nodes are in which component (for later)
        auto& component = strong_components[i];
        for (nid_t node_id : component) {
            component_of[node_id] = i;
        }
        
        // figure out how many times we need to copy this SCC
        
        // wrap the SCC in a handle graph
        SubHandleGraph subgraph(graph);
        for (const nid_t& node_id : component) {
            subgraph.add_handle(graph->get_handle(node_id));
        }
        
        // get a layout with a low FAS
        vector<handle_t> layout = eades_algorithm(&subgraph);
        
        // make sure the layout matches the canonical orientation of the graph
        if (graph->get_is_reverse(layout.front()) != reversed_nodes.count(graph->get_id(layout.front()))) {
            for (int64_t i = 0, j = layout.size() - 1; i < j; i++, j--) {
                auto tmp = layout[i];
                layout[i] = subgraph.flip(layout[j]);
                layout[j] = subgraph.flip(tmp);
            }
            if (layout.size() % 2) {
                layout[layout.size() / 2] = subgraph.flip(layout[layout.size() / 2]);
            }
        }
        
#ifdef debug_dagify
        cerr << "layout for component:" << endl;
        for (const handle_t& h : layout) {
            cerr << "\t" << graph->get_id(h) << (graph->get_is_reverse(h) ? "-" : "+") << endl;
        }
#endif
        
        // record the ordering of the layout so we can identify backward edges
        unordered_map<handle_t, size_t> ordering;
        for (size_t i = 0; i < layout.size(); i++) {
            ordering[layout[i]] = i;
        }
        
        // mark the edges as either forward or backward relative to the layout
        vector<vector<size_t>> forward_edges(layout.size());
        vector<pair<size_t, size_t>> backward_edges;
        subgraph.for_each_edge([&](const edge_t& edge) {
            // get the indices of the edge in the layout, making sure to match
            // the canonical orientation
            size_t i, j;
            auto iter = ordering.find(edge.first);
            if (iter != ordering.end()) {
                i = iter->second;
                j = ordering[edge.second];
            }
            else {
                i = ordering[subgraph.flip(edge.second)];
                j = ordering[subgraph.flip(edge.first)];
            }
            
            // classify the edge as forward or backward
            if (i < j) {
                forward_edges[i].push_back(j);
            }
            else {
                backward_edges.emplace_back(i, j);
            }
            
            // always keep going
            return true;
        });
        
        // check for each node whether we've duplicated the component enough times
        // to preserve its cycles
        
        // dynamic progamming structures that represent distances within the current
        // copy of the SCC and the next copy
        vector<int64_t> distances(layout.size(), numeric_limits<int64_t>::max());
        vector<int64_t> next_distances(layout.size(), numeric_limits<int64_t>::max());
        
        // init the distances so that we are measuring from the end of the heads of
        // backward edges (which cross to the next copy of the SCC)
        for (const pair<size_t, size_t>& bwd_edge : backward_edges) {
            handle_t handle = layout[bwd_edge.first];
            distances[ordering[handle]] = -subgraph.get_length(handle);
        }
        
        // init the tracker that we use for the bail-out condition
        int64_t min_relaxed_dist = -1;
        
        // add copies until the minimum distance to the new copy is longer than the distance we're
        // trying to preserve
        for (size_t copy_num = 0; min_relaxed_dist < int64_t(min_preserved_path_length); copy_num++) {
            
#ifdef debug_dagify
            cerr << "copy number " << copy_num << endl;
#endif
            
            // do we need a new copy of this SCC to preserve paths?
            if (copy_num == injector[layout.front()].size()) {
                // we haven't added this copy of the connected component yet
                
#ifdef debug_dagify
                cerr << "adding nodes for this copy" << endl;
#endif
                
                // add the nodes
                for (const handle_t& original_handle : layout) {
                    // create the node in the same foward orientation as the original
                    handle_t new_handle = into->create_handle(graph->get_sequence(graph->forward(original_handle)));
                    // use the handle locally in the same orientation as it is in the layout
                    if (graph->get_is_reverse(original_handle)) {
                        new_handle = into->flip(new_handle);
                    }
#ifdef debug_dagify
                    cerr << "\t" << graph->get_id(original_handle) << " duplicated to " << into->get_id(new_handle) << endl;
#endif
                    
                    // record the translation between the graphs
                    translator[into->get_id(new_handle)] = graph->get_id(original_handle);
                    injector[original_handle].push_back(new_handle);
                }
                
                // add the forward edges within this copy
                for (size_t i = 0; i < forward_edges.size(); i++) {
                    handle_t from = injector[layout[i]].back();
                    for (const size_t& j : forward_edges[i]) {
                        into->create_edge(from, injector[layout[j]].back());
#ifdef debug_dagify
                        cerr << "\t\tfwd edge " << into->get_id(from) << (into->get_is_reverse(from) ? "-" : "+") << " -> " << into->get_id(injector[layout[j]].back()) << (into->get_is_reverse(injector[layout[j]].back()) ? "-" : "+") << endl;
#endif
                    }
                }
                
                // is there a previous copy?
                if (copy_num > 0) {
                    // add the backward edges between the copies
                    for (const pair<size_t, size_t>& bwd_edge : backward_edges) {
                        const auto& from_copies = injector[layout[bwd_edge.first]];
                        into->create_edge(from_copies[from_copies.size() - 2],
                                          injector[layout[bwd_edge.second]].back());
#ifdef debug_dagify
                        cerr << "\t\tbwd edge " << into->get_id(from_copies[from_copies.size() - 2]) << (into->get_is_reverse(from_copies[from_copies.size() - 2]) ? "-" : "+") << " -> " << into->get_id(injector[layout[bwd_edge.second]].back()) << (into->get_is_reverse(injector[layout[bwd_edge.second]].back()) ? "-" : "+") << endl;
#endif
                    }
                }
            }
            
            // we've finished adding the copy of the SCC that corresponds to this iteration
            // now we will do the dynamic programming to bound the distance to the next SCC
            
            // find the shortest path to the nodes, staying within this copy of the SCC
            for (size_t i = 0; i < distances.size(); i++) {
                // skip infinity to avoid overflow
                if (distances[i] == numeric_limits<int64_t>::max()) {
                    continue;
                }
                
                int64_t dist_thru = distances[i] + subgraph.get_length(layout[i]);
                for (const size_t& j : forward_edges[i]) {
                    distances[j] = min(distances[j], dist_thru);
                }
            }
            
            // now find the minimum distance to nodes in the next copy of the SCC (which
            // may not yet be created in the graph)
            min_relaxed_dist = numeric_limits<int64_t>::max();
            for (const pair<size_t, size_t>& bwd_edge : backward_edges) {
                // skip infinity to avoid overflow
                if (distances[bwd_edge.first] == numeric_limits<int64_t>::max()) {
                    continue;
                }
                
                int64_t dist_thru = distances[bwd_edge.first] + subgraph.get_length(layout[bwd_edge.first]);
                if (dist_thru < next_distances[bwd_edge.second]) {
                    next_distances[bwd_edge.second] = dist_thru;
                    // keep track of the shortest distance to the next copy
                    min_relaxed_dist = min(min_relaxed_dist, dist_thru);
                }
            }
            
#ifdef debug_dagify
            cerr << "distances within component" << endl;
            for (size_t i = 0; i < distances.size(); i++) {
                cerr << "\t" << graph->get_id(layout[i]) << (graph->get_is_reverse(layout[i]) ? "-" : "+") << " " << distances[i] << endl;
            }
            cerr << "distances to next component" << endl;
            for (size_t i = 0; i < next_distances.size(); i++) {
                cerr << "\t" << graph->get_id(layout[i]) << (graph->get_is_reverse(layout[i]) ? "-" : "+") << " " << next_distances[i] << endl;
            }
#endif
            
            // initialize the DP structures for the next iteration
            distances = std::move(next_distances);
            next_distances.assign(distances.size(), numeric_limits<int64_t>::max());
        }
    }
    
#ifdef debug_dagify
    cerr << "adding edges between SCCs" << endl;
#endif
    
    // add edges between the strongly connected components
    graph->for_each_edge([&](const edge_t& canonical_edge) {
        if (component_of[graph->get_id(canonical_edge.first)] != component_of[graph->get_id(canonical_edge.second)]) {
            // this edge is between SCCs
            
            // put the edge in the order of the orientation we've imposed on the graph so
            // we can index into the look structures we created
            edge_t edge = (graph->get_is_reverse(canonical_edge.first) != reversed_nodes.count(graph->get_id(canonical_edge.first)) ?
                           edge_t(graph->flip(canonical_edge.second), graph->flip(canonical_edge.first)) :
                           canonical_edge);
            
            // connect the last copy of the first node to all copies of the second
            // TODO: Why the asymmetry?
            const handle_t& from = injector[edge.first].back();
            for (const handle_t& to : injector[edge.second]) {
                into->create_edge(from, to);
#ifdef debug_dagify
                cerr << "\t" << into->get_id(from) << (into->get_is_reverse(from) ? "-" : "+") << " -> " << into->get_id(to) << (into->get_is_reverse(to) ? "-" : "+") << endl;
#endif
            }
        }
        
        // always keep going
        return true;
    });
    
    // return the ID translator and the handle injector
    return {translator, injector};
}

unordered_map<nid_t, nid_t> dagify(const HandleGraph* graph, MutableHandleGraph* into,
                                   size_t min_preserved_path_length) {
    
    return dagify_internal(graph, into, min_preserved_path_length).first;
}

std::pair<std::unordered_map<nid_t, nid_t>, std::vector<handle_t>> dagify_from(const HandleGraph* graph,
                                                                               std::vector<handle_t> start_handles,
                                                                               DeletableHandleGraph* into,
                                                                               size_t min_preserved_path_length) {
    
    // Cheating implementation
    
    // Dagify the *entire* graph, creating some nodes not reachable from the starting points.
    auto dagification_result = dagify_internal(graph, into, min_preserved_path_length);
    auto& new_id_to_old_id = dagification_result.first;
    auto& old_handle_to_new_handles = dagification_result.second;
    
    // Find all the node IDs we have start handles on
    std::unordered_set<nid_t> start_nodes;
    for (auto& h : start_handles) {
        start_nodes.insert(graph->get_id(h));
    }
    
    // Translate all our start handles into the into graph to get their copies
    // that have all paths going right from their starts.
    std::vector<handle_t> into_start_handles;
    for (auto& h : start_handles) {
        auto found = old_handle_to_new_handles.find(h);
        if (found == old_handle_to_new_handles.end()) {
            // We must be holding this handle in the opposite orientation from
            // its assigned one.
            
            // It must exist in the other orientation.
            auto& copies = old_handle_to_new_handles.at(graph->flip(h));

            // We want all paths to the right, so we need to find the last copy
            // (so we have all paths to the left) and flip it.
            into_start_handles.push_back(into->flip(copies.back()));
        } else {
            // We are holding this handle in the assigned orientation. We want
            // all paths going right and that's what we have from the first
            // copy.
            auto& copies = found->second;
            into_start_handles.push_back(copies.front());
        }
    }
    
    // Tag all the nodes we can reach on oriented walks from the starting
    // handles' copies.
    std::unordered_set<nid_t> visited_nodes;
    handlegraph::algorithms::internal::dfs(
        *into,
        [&](const handle_t& h) -> void {
            // Called when node orientation is first encountered.
            // Mark the node as reachable.
            visited_nodes.insert(into->get_id(h));
        },
        [](const handle_t& ignored) -> void {},
        into_start_handles,
        {}
    );
    
    // Drop all the un-tagged nodes.

    // TODO: We're supposed to be able to destroy the current handle as we loop
    // over it, but HashGraph can't handle that.
    std::vector<handle_t> to_remove;
    into->for_each_handle([&](const handle_t& h) {
        auto node_id = into->get_id(h);
        if (!visited_nodes.count(node_id)) {
            // Destroy each handle not visited on such a walk.
            to_remove.push_back(h);
            // And the translation from it
            new_id_to_old_id.erase(node_id);
        }
    });
    for (auto& h : to_remove) {
         into->destroy_handle(h);
    }
    
    return {new_id_to_old_id, into_start_handles};
}

}
}

