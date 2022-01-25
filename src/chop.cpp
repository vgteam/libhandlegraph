/**
 * \file unchop.cpp
 *
 * Defines an algorithm to join adjacent handles.
 */

#include "handlegraph/algorithms/chop.hpp"
#include "handlegraph/util.hpp"

#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <set>
#include <sstream>
#include <utility>
#include <cassert>
#include <algorithm>

namespace handlegraph {
namespace algorithms {

using namespace std;

/// Concatenates the nodes into a new node with the same external linkage as
/// the provided component. All handles must be in left to right order and a
/// consistent orientation. All paths present must run all the way through the
/// run of nodes from start to end or end to start.
///
/// Returns the handle to the newly created node in the new graph.
///
/// After calling this on a vg::VG, paths will be invalid until
/// Paths::compact_ranks() is called.
handle_t concat_nodes(MutablePathDeletableHandleGraph& graph, const std::vector<handle_t>& nodes) {
    
    // Make sure we have at least 2 nodes
    assert(!nodes.empty() && nodes.front() != nodes.back());
    
    // We also require no edges enter or leave the run of nodes, but we can't check that now.
    
    // Make the new node
    handle_t new_node;
    {
        std::stringstream ss;
        for (auto& n : nodes) {
            ss << graph.get_sequence(n);
        }
        
        new_node = graph.create_handle(ss.str());
    }
    
#ifdef debug
    std::cerr << "Concatenating ";
    for (auto& n : nodes) {
        std::cerr << graph.get_id(n) << (graph.get_is_reverse(n) ? "-" : "+") << " ";
    }
    std::cerr << "into " << graph.get_id(new_node) << "+" << std::endl;
#endif
    
    // We should be able to rely on our handle graph to deduplicate edges, but see https://github.com/vgteam/libbdsg/issues/39
    // So we deduplicate ourselves.
    
    // Find all the neighbors. Make sure to translate edges to the other end of
    // the run, or self loops.
    std::unordered_set<handle_t> left_neighbors;
    graph.follow_edges(nodes.front(), true, [&](const handle_t& left_neighbor) {
        if (left_neighbor == nodes.back()) {
            // Loop back to the end
            left_neighbors.insert(new_node);
        } else if (left_neighbor == graph.flip(nodes.front())) {
            // Loop back to the front
            left_neighbors.insert(graph.flip(new_node));
        } else {
            // Normal edge
            left_neighbors.insert(left_neighbor);
        }
    });
    
    std::unordered_set<handle_t> right_neighbors;
    graph.follow_edges(nodes.back(), false, [&](const handle_t& right_neighbor) {
        if (right_neighbor == nodes.front()) {
            // Loop back to the front.
            // We will have seen it from the other side, so ignore it here.
        } else if (right_neighbor == graph.flip(nodes.back())) {
            // Loop back to the end
            right_neighbors.insert(graph.flip(new_node));
        } else {
            // Normal edge
            right_neighbors.insert(right_neighbor);
        }
    });
    
    // Make all the edges, now that we can't interfere with edge listing
    for (auto& n : left_neighbors) {
#ifdef debug
        std::cerr << "Creating edge " << graph.get_id(n) << (graph.get_is_reverse(n) ? "-" : "+") << " -> "
        <<  graph.get_id(new_node) << (graph.get_is_reverse(new_node) ? "-" : "+") << std::endl;
#endif
        graph.create_edge(n, new_node);
    }
    for (auto& n : right_neighbors) {
        
#ifdef debug
        std::cerr << "Creating edge " << graph.get_id(new_node) << (graph.get_is_reverse(new_node) ? "-" : "+") << " -> "
        <<  graph.get_id(n) << (graph.get_is_reverse(n) ? "-" : "+") << std::endl;
#endif
        
        graph.create_edge(new_node, n);
    }
    
    {
        // Collect the first and last visits along paths. TODO: this requires
        // walking each path all the way through.
        //
        // This contains the first and last handles in path orientation, and a flag
        // for if the path runs along the reverse strand of our run of nodes.
        std::vector<std::tuple<step_handle_t, step_handle_t, bool>> ranges_to_rewrite;
        
        graph.for_each_step_on_handle(nodes.front(), [&](const step_handle_t& front_step) {
            auto path = graph.get_path_handle_of_step(front_step);
#ifdef debug
            std::cerr << "Consider path " << graph.get_path_name(path) << std::endl;
#endif
            
            // If we don't get the same oriented node as where this step is
            // stepping, we must be stepping on the other orientation.
            bool runs_reverse = (graph.get_handle_of_step(front_step) != nodes.front());
            
            step_handle_t back_step = front_step;
            
            while (graph.get_handle_of_step(back_step) !=
                   (runs_reverse ? graph.flip(nodes.back()) : nodes.back())) {
                // Until we find the step on the path that visits the last node in our run.
                // Go along the path towards where our last node should be, in our forward orientation.
                back_step = runs_reverse ? graph.get_previous_step(back_step) : graph.get_next_step(back_step);
            }
            
            // Now we can record the range to rewrite
            // Make sure to put it into path-forward order
            if (runs_reverse) {
#ifdef debug
                std::cerr << "\tGoing to rewrite between " << graph.get_id(graph.get_handle_of_step(front_step)) << " and " << graph.get_id(graph.get_handle_of_step(back_step)) << " backward" << std::endl;
#endif
                ranges_to_rewrite.emplace_back(back_step, front_step, true);
            } else {
                
#ifdef debug
                std::cerr << "\tGoing to rewrite between " << graph.get_id(graph.get_handle_of_step(front_step)) << " and " << graph.get_id(graph.get_handle_of_step(back_step)) << std::endl;
#endif
                ranges_to_rewrite.emplace_back(front_step, back_step, false);
            }
        });
        
        uint64_t i = 0;
        for (auto& range : ranges_to_rewrite) {
            // Rewrite each range to visit the new node in the appropriate orientation instead of whatever it did before
            // Make sure to advance the end of the range because rewrite is end-exclusive (to allow insert).
            graph.rewrite_segment(std::get<0>(range), std::get<1>(range),
                                  {std::get<2>(range) ? graph.flip(new_node) : new_node});
        }
    }
    
    // Destroy all the old edges
    // We know they only exist to the left and right neighbors, and along the run
    for (auto& n : left_neighbors) {
        graph.destroy_edge(n, nodes.front());
    }
    for (auto& n : right_neighbors) {
        graph.destroy_edge(nodes.back(), n);
    }
    auto it = nodes.begin();
    auto next_it = it;
    ++next_it;
    while (next_it != nodes.end()) {
        graph.destroy_edge(*it, *next_it);
        it = next_it;
        ++next_it;
    }
    
    for (auto& n : nodes) {
        // Destroy all the old nodes
#ifdef debug
        std::cerr << "Destroying node " << graph.get_id(n) << std::endl;
#endif
        graph.destroy_handle(n);
    }
    
    /*
     #ifdef debug
     std::cerr << "Paths after concat: " << std::endl;
     graph.for_each_path_handle([&](const path_handle_t p) {
     std::cerr << graph.get_path_name(p) << ": ";
     for (auto h : graph.scan_path(p)) {
     std::cerr << graph.get_id(h) << (graph.get_is_reverse(h) ? '-' : '+') << " ";
     }
     std::cerr << std::endl;
     });
     #endif
     */
    
    // Return the new handle we merged to.
    return new_node;
}

handle_t combine_handles(MutablePathDeletableHandleGraph& graph,
                         const std::vector<handle_t>& handles) {
    std::string seq;
    for (auto& handle : handles) {
        seq.append(graph.get_sequence(handle));
    }
    handle_t combined = graph.create_handle(seq);
    // relink the inbound and outbound nodes
    // get the edge context
    std::vector<handle_t> edges_fwd_fwd;
    std::vector<handle_t> edges_fwd_rev;
    std::vector<handle_t> edges_rev_fwd;
    std::vector<handle_t> edges_rev_rev;
    graph.follow_edges(
                       handles.back(), false,
                       [&](const handle_t& h) {
        edges_fwd_fwd.push_back(h);
    });
    graph.follow_edges(
                       handles.front(), true,
                       [&](const handle_t& h) {
        edges_fwd_rev.push_back(h);
    });
    // destroy the old handles
    for (auto& handle : handles) {
        graph.destroy_handle(handle);
    }
    // connect the ends to the previous context
    // check that we're not trying to make edges that connect back with the nodes in the component
    // there are three cases
    // self looping, front and rear inverting
    for (auto& h : edges_fwd_fwd) {
        if (h == handles.front()) {
            graph.create_edge(combined, combined);
        } else if (h == graph.flip(handles.back())) {
            graph.create_edge(combined, graph.flip(combined));
        } else {
            graph.create_edge(combined, h);
        }
    }
    for (auto& h : edges_fwd_rev) {
        if (h == handles.back()) {
            graph.create_edge(combined, combined);
        } else if (h == graph.flip(handles.front())) {
            graph.create_edge(graph.flip(combined), combined);
        } else {
            graph.create_edge(h, combined);
        }
    }
    return combined;
}

/// Return true if nodes share all paths and the mappings they share in these paths
/// are adjacent, in the specified relative order and orientation.
bool nodes_are_perfect_path_neighbors(const PathHandleGraph& graph, handle_t left_handle, handle_t right_handle) {
    
#ifdef debug_perfect_neighbors
    std::cerr << "Check if " << graph.get_id(left_handle) << (graph.get_is_reverse(left_handle) ? "-" : "+") << " and "
    << graph.get_id(right_handle) << (graph.get_is_reverse(right_handle) ? "-" : "+") << " are perfect path neighbors" << std::endl;
#endif
    
    // Set this false if we find an impermissible step
    bool ok = true;
    
    // Count the number of permissible steps on the next node we find
    size_t expected_next = 0;
    
    graph.for_each_step_on_handle(left_handle, [&](const step_handle_t& here) {
        // For each path step on the left
        
        // We need to work out if the path traverses this handle backward.
        bool step_is_to_reverse_of_handle = (graph.get_handle_of_step(here) != left_handle);
        
#ifdef debug_perfect_neighbors
        std::cerr << "Consider visit of path " << graph.get_path_name(graph.get_path_handle_of_step(here))
        << " to " << (step_is_to_reverse_of_handle ? "reverse" : "forward") << " orientation of handle" << std::endl;
#endif
        
        if (!(step_is_to_reverse_of_handle ? graph.has_previous_step(here) : graph.has_next_step(here))) {
            // If there's no visit to the right of this handle, it can't be to the right next place
#ifdef debug_perfect_neighbors
            std::cerr << "Path stops here so no subsequent handle is a perfect path neighbor" << std::endl;
#endif
            ok = false;
            return false;
        }
        
        // Walk along the path whatever direction is forward relative to our left handle.
        step_handle_t step_to_right = step_is_to_reverse_of_handle ? graph.get_previous_step(here) : graph.get_next_step(here);
        handle_t handle_to_right = graph.get_handle_of_step(step_to_right);
        if (step_is_to_reverse_of_handle) {
            // If we had to walk back along the reverse strand of the path, we have to flip where we ended up.
            handle_to_right = graph.flip(handle_to_right);
        }
        
        if (handle_to_right != right_handle) {
            // It goes to the wrong next place
            
#ifdef debug_perfect_neighbors
            std::cerr << "Path goes to the wrong place ("
            << graph.get_id(handle_to_right) << (graph.get_is_reverse(handle_to_right) ? "-" : "+")
            << ") and so these nodes are not perfect path neighbors"  << std::endl;
#endif
            ok = false;
            return false;
        }
        
        // Otherwise, record a step that is allowed to exist on the next handle
        expected_next++;
        return true;
    });
    
    if (!ok) {
        // We found a bad step, or the path stopped.
        return false;
    }
    
    // Now count up the path steps on the right handle
    size_t observed_next = 0;
    graph.for_each_step_on_handle(right_handle, [&](const step_handle_t& ignored) {
#ifdef debug_perfect_neighbors
        std::cerr << "Next node has path " << graph.get_path_name(graph.get_path_handle_of_step(ignored)) << std::endl;
#endif
        observed_next++;
    });
    
#ifdef debug_perfect_neighbors
    if (observed_next != expected_next) {
        std::cerr << "Next node has " << observed_next << " path visits but should have " << expected_next << std::endl;
    }
#endif
    
    // If there are any steps on the right node that weren't accounted for on
    // the left node, fail. Otherwise, succeed.
    return observed_next == expected_next;
    
}

vector<deque<handle_t>> simple_components(const PathHandleGraph& graph, int min_size = 1) {
    
    // go around and establish groupings
    unordered_set<nid_t> seen;
    vector<deque<handle_t>> components;
    graph.for_each_handle([&](const handle_t& n) {
        nid_t n_id = graph.get_id(n);
        
        if (seen.count(n_id)) {
            // We already found this node in a previous component
            return;
        }
        
#ifdef debug
        cerr << "Component based on " << n_id << endl;
#endif
        
        seen.insert(n_id);
        // go left and right through each as far as we have only single edges connecting us
        // to nodes that have only single edges coming in or out
        // that go to other nodes
        deque<handle_t> c;
        // go left
        {
            
            handle_t l = n;
            
            vector<handle_t> prev;
            graph.follow_edges(l, true, [&](const handle_t& h) {
                prev.push_back(h);
            });
            
#ifdef debug
            cerr << "\tLeft: ";
            for (auto& x : prev) {
                cerr << graph.get_id(x) << (graph.get_is_reverse(x) ? '-' : '+')
                << "(" << graph.get_degree(x, false) << " edges right) ";
                graph.follow_edges(x, false, [&](const handle_t& other) {
                    cerr << "(to " << graph.get_id(other) << (graph.get_is_reverse(other) ? '-' : '+') << ") ";
                });
            }
            cerr << endl;
#endif
            while (prev.size() == 1
                   && graph.get_degree(prev.front(), false) == 1) {
                
                // While there's only one node left of here, and one node right of that node...
                auto last = l;
                // Move over left to that node
                l = prev.front();
                // avoid merging if it breaks stored paths
                if (!nodes_are_perfect_path_neighbors(graph, l, last)) {
#ifdef debug
                    cerr << "\tNot perfect neighbors!" << endl;
#endif
                    break;
                }
                // avoid merging if it's already in this or any other component (catch self loops)
                if (seen.count(graph.get_id(l))) {
#ifdef debug
                    cerr << "\tAlready seen!" << endl;
#endif
                    break;
                }
                
                prev.clear();
                graph.follow_edges(l, true, [&](const handle_t& h) {
                    prev.push_back(h);
                });
                
#ifdef debug
                cerr << "\tLeft: ";
                for (auto& x : prev) {
                    cerr << graph.get_id(x) << (graph.get_is_reverse(x) ? '-' : '+')
                    << "(" << graph.get_degree(x, false) << " edges right) ";
                    graph.follow_edges(x, false, [&](const handle_t& other) {
                        cerr << "(to " << graph.get_id(other) << (graph.get_is_reverse(other) ? '-' : '+') << ") ";
                    });
                }
                cerr << endl;
#endif
                c.push_front(l);
                seen.insert(graph.get_id(l));
            }
        }
        // add the node (in the middle)
        c.push_back(n);
        // go right
        {
            handle_t r = n;
            vector<handle_t> next;
            graph.follow_edges(r, false, [&](const handle_t& h) {
                next.push_back(h);
            });
            
#ifdef debug
            cerr << "\tRight: ";
            for (auto& x : next) {
                cerr << graph.get_id(x) << (graph.get_is_reverse(x) ? '-' : '+')
                << "(" << graph.get_degree(x, true) << " edges left) ";
                graph.follow_edges(x, true, [&](const handle_t& other) {
                    cerr << "(to " << graph.get_id(other) << (graph.get_is_reverse(other) ? '-' : '+') << ") ";
                });
            }
            cerr << endl;
#endif
            while (next.size() == 1
                   && graph.get_degree(next.front(), true) == 1) {
                
                // While there's only one node right of here, and one node left of that node...
                auto last = r;
                // Move over right to that node
                r = next.front();
                // avoid merging if it breaks stored paths
                if (!nodes_are_perfect_path_neighbors(graph, last, r)) {
#ifdef debug
                    cerr << "\tNot perfect neighbors!" << endl;
#endif
                    break;
                }
                // avoid merging if it's already in this or any other component (catch self loops)
                if (seen.count(graph.get_id(r))) {
#ifdef debug
                    cerr << "\tAlready seen!" << endl;
#endif
                    break;
                }
                
                next.clear();
                graph.follow_edges(r, false, [&](const handle_t& h) {
                    next.push_back(h);
                });
                
#ifdef debug
                cerr << "\tRight: ";
                for (auto& x : next) {
                    cerr << graph.get_id(x) << (graph.get_is_reverse(x) ? '-' : '+')
                    << "(" << graph.get_degree(x, true) << " edges left) ";
                    graph.follow_edges(x, true, [&](const handle_t& other) {
                        cerr << "(to " << graph.get_id(other) << (graph.get_is_reverse(other) ? '-' : '+') << ") ";
                    });
                }
                cerr << endl;
#endif
                c.push_back(r);
                seen.insert(graph.get_id(r));
            }
        }
        if (c.size() >= min_size) {
            components.emplace_back(std::move(c));
        }
    });
#ifdef debug
    cerr << "components " << endl;
    for (auto& c : components) {
        for (auto x : c) {
            cerr << graph.get_id(x) << (graph.get_is_reverse(x) ? '-' : '+') << " ";
        }
        cerr << endl;
    }
#endif
    return components;
}

/// Concatenates the nodes into a new node with the same external linkage as
/// the provided component. All handles must be in left to right order and a
/// consistent orientation. All paths present must run all the way through the
/// run of nodes from start to end or end to start.
///
/// Returns the handle to the newly created node.
///
/// After calling this on a vg::VG, paths will be invalid until
/// Paths::compact_ranks() is called.
static handle_t concat_nodes(MutablePathDeletableHandleGraph& graph, const deque<handle_t>& nodes) {
    
    // Make sure we have at least 2 nodes
    assert(!nodes.empty() && nodes.front() != nodes.back());
    
#ifdef debug
    cerr << "Paths before concat: " << endl;
    
    graph.for_each_path_handle([&](const path_handle_t p) {
        cerr << graph.get_path_name(p) << ": ";
        for (auto h : graph.scan_path(p)) {
            cerr << graph.get_id(h) << (graph.get_is_reverse(h) ? '-' : '+') << " ";
        }
        cerr << endl;
    });
    
#endif
    
    // We also require no edges enter or leave the run of nodes, but we can't check that now.
    
    // Make the new node
    handle_t new_node;
    {
        stringstream ss;
        for (auto& n : nodes) {
            ss << graph.get_sequence(n);
        }
        
        new_node = graph.create_handle(ss.str());
    }
    
#ifdef debug
    cerr << "Concatenating ";
    for (auto& n : nodes) {
        cerr << graph.get_id(n) << (graph.get_is_reverse(n) ? "-" : "+") << " ";
    }
    cerr << "into " << graph.get_id(new_node) << "+" << endl;
#endif
    
    // We should be able to rely on our handle graph to deduplicate edges, but see https://github.com/vgteam/libbdsg/issues/39
    // So we deduplicate ourselves.
    
    // Find all the neighbors. Make sure to translate edges to the other end of
    // the run, or self loops.
    unordered_set<handle_t> left_neighbors;
    graph.follow_edges(nodes.front(), true, [&](const handle_t& left_neighbor) {
        if (left_neighbor == nodes.back()) {
            // Loop back to the end
            left_neighbors.insert(new_node);
        } else if (left_neighbor == graph.flip(nodes.front())) {
            // Loop back to the front
            left_neighbors.insert(graph.flip(new_node));
        } else {
            // Normal edge
            left_neighbors.insert(left_neighbor);
        }
    });
    
    unordered_set<handle_t> right_neighbors;
    graph.follow_edges(nodes.back(), false, [&](const handle_t& right_neighbor) {
        if (right_neighbor == nodes.front()) {
            // Loop back to the front.
            // We will have seen it from the other side, so ignore it here.
        } else if (right_neighbor == graph.flip(nodes.back())) {
            // Loop back to the end
            right_neighbors.insert(graph.flip(new_node));
        } else {
            // Normal edge
            right_neighbors.insert(right_neighbor);
        }
    });
    
    // Make all the edges, now that we can't interfere with edge listing
    for (auto& n : left_neighbors) {
#ifdef debug
        cerr << "Creating edge " << graph.get_id(n) << (graph.get_is_reverse(n) ? "-" : "+") << " -> "
        <<  graph.get_id(new_node) << (graph.get_is_reverse(new_node) ? "-" : "+") << endl;
#endif
        graph.create_edge(n, new_node);
    }
    for (auto& n : right_neighbors) {
        
#ifdef debug
        cerr << "Creating edge " << graph.get_id(new_node) << (graph.get_is_reverse(new_node) ? "-" : "+") << " -> "
        <<  graph.get_id(n) << (graph.get_is_reverse(n) ? "-" : "+") << endl;
#endif
        
        graph.create_edge(new_node, n);
    }
    
    {
        // Collect the first and last visits along paths. TODO: this requires
        // walking each path all the way through.
        //
        // This contains the first and last handles in path orientation, and a flag
        // for if the path runs along the reverse strand of our run of nodes.
        vector<tuple<step_handle_t, step_handle_t, bool>> ranges_to_rewrite;
        
        graph.for_each_step_on_handle(nodes.front(), [&](const step_handle_t& front_step) {
            auto path = graph.get_path_handle_of_step(front_step);
#ifdef debug
            cerr << "Consider path " << graph.get_path_name(path) << endl;
#endif
            
            // If we don't get the same oriented node as where this step is
            // stepping, we must be stepping on the other orientation.
            bool runs_reverse = (graph.get_handle_of_step(front_step) != nodes.front());
            
            step_handle_t back_step = front_step;
            
            while(graph.get_handle_of_step(back_step) != (runs_reverse ? graph.flip(nodes.back()) : nodes.back())) {
                // Until we find the step on the path that visits the last node in our run.
                // Go along the path towards where our last node should be, in our forward orientation.
                back_step = runs_reverse ? graph.get_previous_step(back_step) : graph.get_next_step(back_step);
            }
            
            // Now we can record the range to rewrite
            // Make sure to put it into path-forward order
            if (runs_reverse) {
#ifdef debug
                cerr << "\tGoing to rewrite between " << graph.get_id(graph.get_handle_of_step(front_step)) << " and " << graph.get_id(graph.get_handle_of_step(back_step)) << " backward" << endl;
#endif
                ranges_to_rewrite.emplace_back(back_step, front_step, true);
            } else {
                
#ifdef debug
                cerr << "\tGoing to rewrite between " << graph.get_id(graph.get_handle_of_step(front_step)) << " and " << graph.get_id(graph.get_handle_of_step(back_step)) << endl;
#endif
                ranges_to_rewrite.emplace_back(front_step, back_step, false);
            }
        });
        
        for (auto& range : ranges_to_rewrite) {
            // Rewrite each range to visit the new node in the appropriate orientation instead of whatever it did before
            // Make sure to advance the end of the range because rewrite is end-exclusive (to allow insert).
            graph.rewrite_segment(get<0>(range), graph.get_next_step(get<1>(range)), {get<2>(range) ? graph.flip(new_node) : new_node});
        }
    }
    
    for (auto& n : nodes) {
        // Destroy all the old nodes
#ifdef debug
        cerr << "Destroying node " << graph.get_id(n) << endl;
#endif
        graph.destroy_handle(n);
    }
    
#ifdef debug
    cerr << "Paths after concat: " << endl;
    
    graph.for_each_path_handle([&](const path_handle_t p) {
        cerr << graph.get_path_name(p) << ": ";
        for (auto h : graph.scan_path(p)) {
            cerr << graph.get_id(h) << (graph.get_is_reverse(h) ? '-' : '+') << " ";
        }
        cerr << endl;
    });
    
#endif
    
    // Return the new handle we merged to.
    return new_node;
}

void unchop(MutablePathDeletableHandleGraph& graph) {
#ifdef debug
    std::cerr << "Running unchop" << std::endl;
#endif
    
    std::unordered_map<nid_t, uint64_t> node_rank;
    uint64_t rank = 0;
    graph.for_each_handle([&](const handle_t& h) {
        node_rank[graph.get_id(h)] = rank++;
    });
    
    // if possible, don't hold these in memory
    // meanwhile, we exploit the it for parallelizing the paths' validation
    std::vector<std::string> path_names;
    path_names.resize(graph.get_path_count());
    
    std::vector<std::string> path_sequences;
    path_sequences.resize(graph.get_path_count());
    
    rank = 0;
    graph.for_each_path_handle([&](const path_handle_t& p) {
        path_names[rank] = graph.get_path_name(p);
        
        auto& seq = path_sequences[rank];
        graph.for_each_step_in_path(p, [&](const step_handle_t& s) {
            seq.append(graph.get_sequence(graph.get_handle_of_step(s)));
        });
        
        ++rank;
    });
    
    auto components = simple_components(graph, 2);
    std::unordered_set<nid_t> to_merge;
    for (auto& comp : components) {
        for (auto& handle : comp) {
            to_merge.insert(graph.get_id(handle));
        }
    }
    std::vector<std::pair<double, handle_t>> ordered_handles;
    graph.for_each_handle([&](const handle_t& handle) {
        if (!to_merge.count(graph.get_id(handle))) {
            ordered_handles.push_back(std::make_pair(node_rank[graph.get_id(handle)],
                                                     handle));
        }
    });
    
    uint64_t num_node_unchopped = 0;
    uint64_t num_new_nodes = 0;
    for (auto& comp : components) {
#ifdef debug
        std::cerr << "Unchop " << comp.size() << " nodes together" << std::endl;
#endif
        if (comp.size() >= 2) {
            // sort by lowest rank to maintain order
            double rank_sum = 0;
            for (auto& handle : comp) {
                rank_sum += node_rank[graph.get_id(handle)];
            }
            double rank_v = rank_sum / comp.size();
            handle_t n = concat_nodes(graph, comp);
            ordered_handles.push_back(std::make_pair(rank_v, n));
            //node_order.push_back(graph.get_id(n));
            num_node_unchopped += comp.size();
            num_new_nodes++;
        } else {
            for (auto& c : comp) {
                ordered_handles.push_back(std::make_pair(node_rank[graph.get_id(c)], c));
            }
        }
    }
    
    assert(graph.get_node_count() == ordered_handles.size());
    
    sort(ordered_handles.begin(), ordered_handles.end(),
         [](const pair<double, handle_t>& a, const pair<double, handle_t>& b) {
        return a.first < b.first || (a.first == b.first && as_integer(a.second) < as_integer(b.second));
    });
    
    std::vector<handle_t> handle_order;
    for (auto& h : ordered_handles) {
        handle_order.push_back(h.second);
    }
    
    graph.apply_ordering(handle_order, true);
}

static void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length, const std::function<void(nid_t, size_t, size_t, handle_t)>* record_change) {
    // borrowed from https://github.com/vgteam/odgi/blob/master/src/subcommand/chop_main.cpp
    
    std::vector<std::tuple<uint64_t, uint64_t, handle_t>> originalRank_inChoppedNodeRank_handle;
    std::vector<std::pair<uint64_t, handle_t>> originalRank_handleToChop;
    std::vector<nid_t> originalId;
    uint64_t rank = 0;
    graph.for_each_handle([&](const handle_t& handle) {
        if (graph.get_length(handle) > max_node_length) {
            originalRank_handleToChop.push_back(std::make_pair(rank, handle));
        } else {
            originalRank_inChoppedNodeRank_handle.push_back(std::make_tuple(rank, 0, handle));
        }
        
        if (record_change) {
            // We'll need the ID this original node had, for emitting chops and renumbers
            originalId.push_back(graph.get_id(handle));
        }
        
        rank++;
    });
    
    if (originalRank_handleToChop.empty()) {
        // No node is long enough to chop. Do nothing.
        return;
    }
    
    for (auto rank_handle : originalRank_handleToChop) {
        // get divide points
        uint64_t length = graph.get_length(rank_handle.second);
        std::vector<size_t> offsets;
        for (uint64_t i = max_node_length; i < length; i += max_node_length) {
            offsets.push_back(i);
        }
        
        rank = 0;
        for (auto chopped_handle : graph.divide_handle(rank_handle.second, offsets)) {
            originalRank_inChoppedNodeRank_handle.emplace_back(rank_handle.first, rank, chopped_handle);
            
            rank++;
        }
    }
    
    sort(originalRank_inChoppedNodeRank_handle.begin(), originalRank_inChoppedNodeRank_handle.end());
    
    std::vector<handle_t> new_handles;
    for (auto x_y_z : originalRank_inChoppedNodeRank_handle) {
        new_handles.push_back(std::get<2>(x_y_z));
    }
    
    bool idsChanged = graph.apply_ordering(new_handles, true);
    
    // Define a getter to get new node handle by new node rank.
    auto get_handle_for_new_rank = [&](size_t rank) {
        if (idsChanged) {
            // Handles were invalidated but everything was renumbered by rank.
            return graph.get_handle((nid_t)(rank + 1), false);
        } else {
            // Stored new handles are valid still.
            return get<2>(originalRank_inChoppedNodeRank_handle[rank]);
        }
    };
    
    if (record_change) {
        // We need to announce our changes
        // Nodes are now numbered 1 to n in correspondence with
        // originalRank_inChoppedNodeRank_handle.
        // So we need to walk them together and look at the node lengths,
        // and generate calls to record_change for all the segments (or at
        // least those that aren't full-length and changed number).
        
        // Where are we aloing the current original node's forward strand?
        size_t offset = 0;
        // And where do we start on its reverse strand?
        size_t revOffset = 0;
        // What handles are the handles for the new nodes that make up this original node, from here to the end?
        std::deque<handle_t> pieces;
        // We set this if the original node was actually split
        bool originalSplit = false;
        // We set this to the rank of the original node we are at.
        size_t originalRank = 0;
        for (size_t newRank = 0; newRank < originalRank_inChoppedNodeRank_handle.size(); newRank++) {
            if (pieces.empty()) {
                // We have started a new original node.
                // Grab its rank
                originalRank = get<0>(originalRank_inChoppedNodeRank_handle[newRank]);
                // Reset to offset 0.
                offset = 0;
                std::cerr << "Original rank " << originalRank << " starts at new rank " << newRank << std::endl;
                revOffset = 0;
                // And scan to the end of the original node to get our reverse
                // strand offset and populate pieces.
                // The reverse offset has to have the first handle's length in
                // it, because the loop invariant is shifted to save a second
                // length call on each subsequent loop.
                size_t nextOriginalStarts = newRank;
                while (nextOriginalStarts < originalRank_inChoppedNodeRank_handle.size() && get<0>(originalRank_inChoppedNodeRank_handle[nextOriginalStarts]) == originalRank) {
                    // Until we hit a new node that belongs to a different old node, keep accumulating handles in pieces.
                    handle_t cursorHandle = get_handle_for_new_rank(nextOriginalStarts);
                    pieces.push_back(cursorHandle);
                    std::cerr << "New rank " << nextOriginalStarts << " is new ID " << graph.get_id(cursorHandle) << std::endl;
                    // And add its length to the offset from the end of the original node
                    revOffset += graph.get_length(cursorHandle);
                    std::cerr << "And has length " << graph.get_length(cursorHandle) << std::endl;
                    // Then look at the next new node.
                    nextOriginalStarts++;
                }
                
                // Decide if this original node actually got split.
                originalSplit = (pieces.size() > 1);
            }
            
            std::cerr << "So we're at offset " << offset << " forward and " << revOffset << " reverse for total original length " << (offset + revOffset) << std::endl;
            
            // Then look at what's at the front of pieces, which is going to be us.
            handle_t newHandle = pieces.front();
            size_t length = graph.get_length(newHandle);
            
            // Compute the right revOfset, which should no longer include our length.
            // Its loop invariant is weird to save a length check.
            revOffset -= length;
            
            std::cerr << "So we remove the " << length << " of node " << graph.get_id(newHandle) << " and we have a rev offset of " << revOffset << endl;
            
            if (idsChanged || originalSplit) {
                // We are (probably) an important change.
                // TODO: Elide cases where IDs changed but this ID didn't.
                // Announce a new node starting here
                (*record_change)(originalId[originalRank], offset, revOffset, newHandle);
            }
            
            // Update forward strand offset for next piece of original node
            offset += length;
            
            std::cerr << "And then we add it on the forward strand and we have a forward offset for the next node of " << offset << endl;
            
            // And advance to next piece
            pieces.pop_front();
        }
    }
}

void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length, const std::function<void(nid_t, size_t, size_t, handle_t)>& record_change) {
    chop(graph, max_node_length, &record_change);
}

void chop(MutablePathDeletableHandleGraph& graph, size_t max_node_length) {
    chop(graph, max_node_length, nullptr);
}


}
}

