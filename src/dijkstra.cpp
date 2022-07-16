/**
 * \file dijkstra.cpp
 *
 * Implementation of Dijkstra's Algorithm over the bidirected graph.
 */

#include "handlegraph/algorithms/dijkstra.hpp"

#include <queue>

namespace handlegraph {
namespace algorithms {

using namespace std;

bool dijkstra(const HandleGraph* g, handle_t start,
              function<bool(const handle_t&, size_t)> reached_callback,
              bool traverse_leftward) {
              
    unordered_set<handle_t> starts;
    starts.insert(start);          
    
    // Implement single-start search in terms of multi-start search
    return dijkstra(g, starts, reached_callback, traverse_leftward);
              
}

bool dijkstra(const HandleGraph* g, const unordered_set<handle_t>& starts,
              function<bool(const handle_t&, size_t)> reached_callback,
              bool traverse_leftward) {

    // We keep a priority queue so we can visit the handle with the shortest
    // distance next. We put handles in here whenever we see them with shorter
    // distances (since STL priority queue can't update), so we also need to
    // make sure nodes coming out haven't been visited already.
    using Record = pair<size_t, handle_t>;
    
    // We filter out handles that have already been visited.
    unordered_set<handle_t> visited;
    
    // We need a custom ordering for the queue
    struct IsFirstGreater {
        IsFirstGreater() = default;
        ~IsFirstGreater() = default;
        inline bool operator()(const Record& a, const Record& b) {
            return a.first > b.first;
        }
    };
    
    priority_queue<Record, vector<Record>, IsFirstGreater> queue;
    unordered_set<handle_t> dequeued;
    
    // We keep a current handle
    handle_t current;
    size_t distance = 0;
    for (auto& start : starts) {
        queue.push(make_pair(distance, start));
    }
    
    while (!queue.empty()) {
        // While there are things in the queue, get the first.
        tie(distance, current) = queue.top();
        queue.pop();
        if (dequeued.count(current)) {
            continue;
        }
        else {
            dequeued.insert(current);
        }
        
#ifdef debug_vg_algorithms
        cerr << "Visit " << g->get_id(current) << " " << g->get_is_reverse(current) << " at distance " << distance << endl;
#endif    


        // Emit the handle as being at the given distance
        if (!reached_callback(current, distance)) {
            // The user told us to stop. Return that we stopped early.
            
#ifdef debug_vg_algorithms
            cerr << "\tAbort search" << endl;
#endif  
            
            return false;
        }
        
        // Remember that we made it here.
        visited.emplace(current);
        
        if (!starts.count(current)) {
            // Up the distance with the node's length. We don't do this for the
            // start handles because we want to count distance from the *end* of
            // the start handles unless directed otherwise.
            distance += g->get_length(current);
        }
            
        g->follow_edges(current, traverse_leftward, [&](const handle_t& next) {
            // For each handle to the right of here
            
            if (!visited.count(next)) {
                // New shortest distance. Will never happen after the handle comes out of the queue because of Dijkstra.
                queue.push(make_pair(distance, next));
                
#ifdef debug_vg_algorithms
                cerr << "\tNew best path to " << g->get_id(next) << " " << g->get_is_reverse(next)
                    << " at distance " << distance << endl;
#endif
                
            } else {
#ifdef debug_vg_algorithms
                cerr << "\tDisregard path to " << g->get_id(next) << " " << g->get_is_reverse(next)
                    << " at distance " << distance << endl;
#endif
            }
        });
    }

    // If we made it here, we finished the entire graph.
    return true;

}

void for_each_handle_in_shortest_path(const HandleGraph* g, handle_t start, handle_t end, std::function<bool(const handle_t&, size_t)> iteratee) {
#ifdef debug_vg_algorithms
    cerr << "Get shortest path from " << g->get_id(start) << (g->get_is_reverse(start) ? "rev" : "fd") 
         << " to " << g->get_id(end) << (g->get_is_reverse(end) ? "rev" : "fd") << endl;
#endif
    // Run dijktra copy and pasted from above, but record the traceback for each node 

    //Map each handle_t to the handle_t that came before it in the shortest path from start to it and the distance
    unordered_map<handle_t, pair<handle_t, size_t>> traceback_map;

    // We keep a priority queue so we can visit the handle with the shortest
    // distance next. We put handles in here whenever we see them with shorter
    // distances (since STL priority queue can't update), so we also need to
    // make sure nodes coming out haven't been visited already.
    // First handle_t is the node, second is the node that led to it
    using Record = tuple<size_t, handle_t, handle_t>;
    
    // We filter out handles that have already been visited.
    unordered_set<handle_t> visited;
    
    // We need a custom ordering for the queue
    struct IsFirstGreater {
        IsFirstGreater() = default;
        ~IsFirstGreater() = default;
        inline bool operator()(const Record& a, const Record& b) {
            return std::get<0>(a) > std::get<0>(b);
        }
    };
    
    priority_queue<Record, vector<Record>, IsFirstGreater> queue;
    unordered_set<handle_t> dequeued;
    
    // We keep a current handle
    handle_t current;
    size_t distance = 0;
    handle_t backtrace;
    queue.push(make_tuple(distance, start, start));
    
    while (!queue.empty()) {
        // While there are things in the queue, get the first.
        tie(distance, current, backtrace) = queue.top();
        queue.pop();
        if (dequeued.count(current)) {
            continue;
        }
        else {
            dequeued.insert(current);
        }
        //Add this node and its predecessor to the traceback
        traceback_map.emplace(current, make_pair(backtrace, distance));

        if (current == end) {
            //If we find the end node, stop
            break;
        }
        
#ifdef debug_vg_algorithms
        cerr << "Visit " << g->get_id(current) << " " << g->get_is_reverse(current) << " at distance " << distance << endl;
        cerr << "\t adding to backtrace " << g->get_id(backtrace) << " " << g->get_is_reverse(backtrace) << endl;
#endif    



        // Remember that we made it here.
        visited.emplace(current);
        
        if (start != current) {
            // Up the distance with the node's length. We don't do this for the
            // start handles because we want to count distance from the *end* of
            // the start handles unless directed otherwise.
            distance += g->get_length(current);
        }
            
        g->follow_edges(current, false, [&](const handle_t& next) {
            // For each handle to the right of here
            
            if (!visited.count(next)) {
                // New shortest distance. Will never happen after the handle comes out of the queue because of Dijkstra.
                queue.push(make_tuple(distance, next, current));
                
#ifdef debug_vg_algorithms
                cerr << "\tNew best path to " << g->get_id(next) << " " << g->get_is_reverse(next)
                    << " at distance " << distance << endl;
#endif
                
            } else {
#ifdef debug_vg_algorithms
                cerr << "\tDisregard path to " << g->get_id(next) << " " << g->get_is_reverse(next)
                    << " at distance " << distance << endl;
#endif
            }
        });
    }

#ifdef debug_vg_algorithms
cerr << "Traceback to start " << g->get_id(start) << (g->get_is_reverse(start) ? "rev" : "fd")<< endl;
unordered_set<handle_t> seen;
#endif
    //Now traceback maps each handle_t to the distance from start to it, and the thing that came before it in the 
    //shortest path
    //trace back from end to start, keeping a vector of the handles and distances ordered backwards
    vector<pair<handle_t, size_t>> backwards_traceback;
    current = end;
    while (current != start) {

        //Find the thing that preceeds current, and the distance to current
        pair<handle_t, size_t> predecessor = traceback_map[current];

        //Add current to the list
        backwards_traceback.emplace_back(current, predecessor.second);

#ifdef debug_vg_algorithms
cerr << "At " << g->get_id(current) << (g->get_is_reverse(current) ? "rev" : "fd") << " back to " 
     <<  g->get_id(predecessor.first) << (g->get_is_reverse(predecessor.first) ? "rev" : "fd") << endl;
if(seen.count(current)){
    throw runtime_error("Already seen this ");
}
seen.emplace(current);
#endif

        //And move back one step
        current = predecessor.first; 
    }
#ifdef debug_vg_algorithms
    cerr << "Go forwards to get shortest path" << endl;
#endif

    //Go backwards through backwards_traceback and call iteratee on everything, excluding end
    if (backwards_traceback.size() == 0) {
        return;
    }
    for (int i = backwards_traceback.size()-1 ; i >= 1 ; i--) {
        auto& x = backwards_traceback[i];
#ifdef debug_vg_algorithms
cerr << "At " << g->get_id(x.first) << (g->get_is_reverse(x.first) ? "rev" : "fd") << endl; 
#endif
        //run iteratee and stop if it returned false
        if (!iteratee(x.first, x.second)) {
            return;
        }
    }
}

    
}
}
