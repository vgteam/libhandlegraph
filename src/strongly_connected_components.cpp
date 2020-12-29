#include "handlegraph/algorithms/strongly_connected_components.hpp"

#include <unordered_map>
#include <deque>
//#define debug

namespace handlegraph {
namespace algorithms {

using namespace std;

// TODO: I don't love this DFS implementation, which is too confusing to
// be all that repurposeable. Hiding it in the cpp for now.

// depth first search across node traversals with interface to traversal tree via callback
void dfs(
         const HandleGraph& graph,
         const function<void(const handle_t&)>& handle_begin_fn,  // called when node orientation is first encountered
         const function<void(const handle_t&)>& handle_end_fn,    // called when node orientation goes out of scope
         const function<bool(void)>& break_fn,                    // called to check if we should stop the DFS; we stop when true is returned.
         const function<void(const edge_t&)>& edge_fn,            // called when an edge is encountered
         const function<void(const edge_t&)>& tree_fn,            // called when an edge forms part of the DFS spanning tree
         const function<void(const edge_t&)>& edge_curr_fn,       // called when we meet an edge in the current tree component
         const function<void(const edge_t&)>& edge_cross_fn,      // called when we meet an edge in an already-traversed tree component
         const vector<handle_t>& sources,                         // start only at these node traversals
         const unordered_set<handle_t>& sinks                     // when hitting a sink, don't keep walking
) {
    
    // to maintain search state
    enum SearchState { PRE = 0, CURR, POST };
    unordered_map<handle_t, SearchState> state; // implicitly constructed entries will be PRE.
    
    // to maintain stack frames
    struct Frame {
        handle_t handle;
        vector<edge_t>::iterator begin, end;
        Frame(handle_t h,
              vector<edge_t>::iterator b,
              vector<edge_t>::iterator e)
        : handle(h), begin(b), end(e) { }
    };
    
    // maintains edges while the node traversal's frame is on the stack
    unordered_map<handle_t, vector<edge_t> > edges;
    
    // do dfs from given root.  returns true if terminated via break condition, false otherwise
    function<bool(const handle_t&)> dfs_single_source = [&](const handle_t& root) {
        
#ifdef debug
        cerr << "Starting a DFS from " << graph.get_id(root) << " " << graph.get_is_reverse(root) << endl;
#endif
        
        // to store the stack frames
        deque<Frame> todo;
        if (state[root] == SearchState::PRE) {
#ifdef debug
            cerr << "\tMoving from PRE to CURR" << endl;
#endif
            
            state[root] = SearchState::CURR;
            
            // Collect all the edges attached to the outgoing side of the
            // traversal.
            auto& es = edges[root];
            // follow edges?
            graph.follow_edges(root, false, [&](const handle_t& next) {
                es.push_back(graph.edge_handle(root, next));
                
#ifdef debug
                cerr << "\t\tWill need to investigate edge to "
                << graph.get_id(next) << " " << graph.get_is_reverse(next) << endl;
#endif
                
            });
            
            todo.push_back(Frame(root, es.begin(), es.end()));
            // run our discovery-time callback
            handle_begin_fn(root);
            // and check if we should break
            if (break_fn()) {
                return true;
            }
        }
        // now begin the search rooted at this NodeTraversal
        while (!todo.empty()) {
            // get the frame
            auto& frame = todo.back();
            // and set up reference to it
            auto handle = frame.handle;
            auto edges_begin = frame.begin;
            auto edges_end = frame.end;
            todo.pop_back();
            
#ifdef debug
            cerr << "Continuing DFS; at " << graph.get_id(handle) << " " << graph.get_is_reverse(handle) << endl;
#endif
            
            // run through the edges to handle
            while (edges_begin != edges_end) {
                auto& edge = *edges_begin;
                // run the edge callback
                edge_fn(edge);
                
                // what's the handle we'd get to following this edge
                const handle_t& target = graph.traverse_edge_handle(edge, handle);
                
#ifdef debug
                cerr << "\tProcessing edge " << graph.get_id(edge.first) << " " << graph.get_is_reverse(edge.first) << " to "
                << graph.get_id(edge.second) << " " << graph.get_is_reverse(edge.second) << endl;
                cerr << "\tHandled as edge " << graph.get_id(handle) << " " << graph.get_is_reverse(handle) << " to "
                << graph.get_id(target) << " " << graph.get_is_reverse(target) << endl;
#endif
                
                auto search_state = state[target];
                // if we've not seen it, follow it
                if (search_state == SearchState::PRE) {
                    
#ifdef debug
                    cerr << "\t\tDiscovered new destination!" << endl;
#endif
                    
                    tree_fn(edge);
                    // save the rest of the search for this handle on the stack
                    todo.push_back(Frame(handle, ++edges_begin, edges_end));
                    
#ifdef debug
                    cerr << "\t\tPausing existing stack frame (" << (edges_end - edges_begin) << " edges left)" << endl;
#endif
                    
                    // switch our focus to the handle at the other end of the edge
                    handle = target;
                    
#ifdef debug
                    cerr << "\t\tFocusing and moving " << graph.get_id(handle) << " "
                    << graph.get_is_reverse(handle) << " from PRE to CURR" << endl;
#endif
                    
                    // and store it on the stack
                    state[handle] = SearchState::CURR;
                    auto& es = edges[handle];
                    
                    // only walk out of handle that are not the sink
                    if (sinks.empty() || sinks.count(handle) == false) {
                        // follow edges?
                        graph.follow_edges(handle, false, [&](const handle_t& next) {
                            es.push_back(graph.edge_handle(handle, next));
                            
#ifdef debug
                            cerr << "\t\t\tWill need to investigate edge to "
                            << graph.get_id(next) << " " << graph.get_is_reverse(next) << endl;
#endif
                            
                        });
                    }
                    edges_begin = es.begin();
                    edges_end = es.end();
                    // run our discovery-time callback
                    handle_begin_fn(handle);
                    
                    // We will continue with edges_begin and edges_end and handle adjusted to this new loaded stack frame.
                    
#ifdef debug
                    cerr << "Now continuing DFS from " << graph.get_id(handle) << " " << graph.get_is_reverse(handle) << endl;
#endif
                    
                } else if (search_state == SearchState::CURR) {
                    // if it's on the stack
                    
#ifdef debug
                    cerr << "\tDestination already discovered" << endl;
#endif
                    
                    edge_curr_fn(edge);
                    ++edges_begin;
                } else {
                    // it's already been handled, so in another part of the tree
                    
#ifdef debug
                    cerr << "\tDestination already completed by a shorter path" << endl;
#endif
                    
                    edge_cross_fn(edge);
                    ++edges_begin;
                }
            }
            state[handle] = SearchState::POST;
            handle_end_fn(handle);
            edges.erase(handle); // clean up edge cache
        }
        
        return false;
    };
    
    if (sources.empty()) {
        // attempt the search rooted at all NodeTraversals
        graph.for_each_handle([&](const handle_t& handle_fwd) {
            handle_t handle_rev = graph.flip(handle_fwd);
            dfs_single_source(handle_fwd);
            dfs_single_source(handle_rev);
        });
    } else {
        for (auto source : sources) {
            dfs_single_source(source);
        }
    }
}

void dfs(const HandleGraph& graph,
         const function<void(const handle_t&)>& handle_begin_fn,
         const function<void(const handle_t&)>& handle_end_fn,
         const vector<handle_t>& sources,
         const unordered_set<handle_t>& sinks) {
    auto edge_noop = [](const edge_t& e) { };
    dfs(graph,
        handle_begin_fn,
        handle_end_fn,
        [](void) { return false; },
        edge_noop,
        edge_noop,
        edge_noop,
        edge_noop,
        sources,
        sinks);
}

void dfs(const HandleGraph& graph,
         const function<void(const handle_t&)>& handle_begin_fn,
         const function<void(const handle_t&)>& handle_end_fn,
         const function<bool(void)>& break_fn) {
    auto edge_noop = [](const edge_t& e) { };
    vector<handle_t> empty_sources;
    unordered_set<handle_t> empty_sinks;
    dfs(graph,
        handle_begin_fn,
        handle_end_fn,
        break_fn,
        edge_noop,
        edge_noop,
        edge_noop,
        edge_noop,
        empty_sources,
        empty_sinks);
}

// recursion-free version of Tarjan's strongly connected components algorithm
// https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
// Generalized to bidirected graphs as described (confusingly) in
// "Decomposition of a bidirected graph into strongly connected components and
// its signed poset structure", by Kazutoshi Ando, Satoru Fujishige, and Toshio
// Nemoto. http://www.sciencedirect.com/science/article/pii/0166218X95000683

// The best way to think about that paper is that the edges are vectors in a
// vector space with number of dimensions equal to the number of nodes in the
// graph, and an edge attaching to the end a node is the positive unit vector in
// its dimension, and an edge attaching to the start of node is the negative
// unit vector in its dimension.

// The basic idea is that you just consider the orientations as different nodes,
// and the edges as existing between both pairs of orientations they connect,
// and do connected components on that graph. Since we don't care about
// "consistent" or "inconsistent" strongly connected components, we just put a
// node in a component if either orientation is in it. But bear in mind that
// both orientations of a node might not actually be in the same strongly
// connected component in a bidirected graph, so now the components may overlap.
vector<unordered_set<nid_t>> strongly_connected_components(const HandleGraph* handle_graph) {
    
#ifdef debug
    cerr << "Computing strongly connected components" << endl;
#endif
    
    // What node visit step are we on?
    int64_t index = 0;
    // What's the search root from which a node was reached?
    unordered_map<handle_t, handle_t> roots;
    // At what index step was each node discovered?
    unordered_map<handle_t, int64_t> discover_idx;
    // We need our own copy of the DFS stack
    vector<handle_t> stack;
    // And our own set of nodes already on the stack
    unordered_set<handle_t> on_stack;
    // What components did we find? Because of the way strongly connected
    // components generalizes, both orientations of a node always end up in the
    // same component.
    vector<unordered_set<nid_t>> components;
    
    // A single node ID from each component we've already added, which we use
    // to deduplicate the results
    // TODO: why do we produce duplicate components in the first place?
    unordered_set<nid_t> already_used;
    
    dfs(*handle_graph,
        [&](const handle_t& trav) {
        // When a NodeTraversal is first visited
#ifdef debug
        cerr << "First visit to " << handle_graph->get_id(trav) << " orientation " << handle_graph->get_is_reverse(trav) << endl;
#endif
        // It is its own root
        roots[trav] = trav;
        // We discovered it at this step
        discover_idx[trav] = index++;
        // And it's on the stack
        stack.push_back(trav);
        on_stack.insert(trav);
    },
        [&](const handle_t& trav) {
        // When a NodeTraversal is done being recursed into
#ifdef debug
        cerr << "Finishing " << handle_graph->get_id(trav) << " orientation " << handle_graph->get_is_reverse(trav) << endl;
#endif
        // Go through all the NodeTraversals reachable reading onwards from this traversal.
        handle_graph->follow_edges(trav, false, [&](const handle_t& next) {
#ifdef debug
            cerr << "\tCould next reach " << handle_graph->get_id(next) << " orientation " << handle_graph->get_is_reverse(next) << endl;
#endif
            if (on_stack.count(next)) {
                // If any of those NodeTraversals are on the stack already
#ifdef debug
                cerr << "\t\tIt is already on the stack, so maybe we want its root" << endl;
#endif
                auto& node_root = roots[trav];
                auto& next_root = roots[next];
#ifdef debug
                cerr << "\t\t\tWe have root " << handle_graph->get_id(node_root) << " orientation "
                << handle_graph->get_is_reverse(node_root)
                << " discovered at time " << discover_idx[node_root] << endl;
                cerr << "\t\t\tThey have root " << handle_graph->get_id(next_root) << " orientation "
                << handle_graph->get_is_reverse(next_root)
                << " discovered at time " << discover_idx[next_root] << endl;
#endif
                // Adopt the root of the NodeTraversal that was discovered first.
                roots[trav] = discover_idx[node_root] < discover_idx[next_root] ?
                node_root : next_root;
#ifdef debug
                cerr << "\t\t\tWinning root: " << handle_graph->get_id(roots[trav]) << " orientation "
                << handle_graph->get_is_reverse(roots[trav]) << endl;
#endif
            }
            return true;
        });
        
        if (roots[trav] == trav) {
            // If we didn't find a better root
#ifdef debug
            cerr << "\tWe are our own best root, so glom up everything under us" << endl;
#endif
            handle_t other;
            bool is_duplicate = false;
            unordered_set<nid_t> component;
            do
            {
                // Grab everything that was put on the DFS stack below us
                // and put it in our component.
                other = stack.back();
                stack.pop_back();
                on_stack.erase(other);
                
                nid_t node_id = handle_graph->get_id(other);
                
                if (already_used.count(node_id)) {
                    is_duplicate = true;
                    break;
                }
                
                component.insert(node_id);
#ifdef debug
                cerr << "\t\tSnarf up node " << handle_graph->get_id(other) << " from handle in orientation "
                << handle_graph->get_is_reverse(other) << endl;
#endif
            } while (other != trav);
            
            if (!is_duplicate) {
                // use one node ID to mark this component as finished
                already_used.insert(*component.begin());
                // add it to the return valuse
                components.emplace_back(move(component));
            }
        }
    },
        vector<handle_t>(), unordered_set<handle_t>());
    
    return components;
}

}
}
