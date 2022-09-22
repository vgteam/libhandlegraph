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

//#define debug_vg_algorithms

bool dijkstra(const HandleGraph* g, handle_t start,
              function<bool(const handle_t&, size_t)> reached_callback,
              bool traverse_leftward, bool prune, bool cycle_to_start) {
              
    unordered_set<handle_t> starts;
    starts.insert(start);          
    
    // Implement single-start search in terms of multi-start search
    return dijkstra(g, starts, reached_callback, traverse_leftward, prune, cycle_to_start);
              
}

bool dijkstra(const HandleGraph* g, const unordered_set<handle_t>& starts,
              function<bool(const handle_t&, size_t)> reached_callback,
              bool traverse_leftward, bool prune, bool cycle_to_start) {

#ifdef debug_vg_algorithms
    cerr << "Doing Dijkstra traversal from " << starts.size() << " start points, "
        << (traverse_leftward ? "left" : "right")
        << ", with pruning " << (prune ? "on" : "off")
        << " and cycle visits to starts " << (cycle_to_start ? "on" : "off") << endl;
#endif

    // We keep a priority queue so we can visit the handle with the shortest
    // distance next. We put handles in here whenever we see them with shorter
    // distances (since STL priority queue can't update), so we also need to
    // make sure nodes coming out haven't been visited already.
    using Record = pair<size_t, handle_t>;
    
    // We filter out handles that have already been visited.
    unordered_set<handle_t> visited;
    
    // We need to know if we stopped early
    bool stopped_early = false;
    
    // And for allowing visiting the starts by cycle, we need to discount the first time we see them, at distance 0
    unordered_set<handle_t> unseen_starts;
    if (cycle_to_start) {
        unseen_starts = starts;
    }
    
    // We need a custom ordering for the queue
    struct IsFirstGreater {
        IsFirstGreater() = default;
        ~IsFirstGreater() = default;
        inline bool operator()(const Record& a, const Record& b) {
            return a.first > b.first;
        }
    };
    
    priority_queue<Record, vector<Record>, IsFirstGreater> queue;
    
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
        
        if (cycle_to_start && unseen_starts.count(current)) {
            // This is the very first visit to this start, so don't count it as
            // visited.
#ifdef debug_vg_algorithms
            cerr << "Expand start " << g->get_id(current) << " " << g->get_is_reverse(current) << " at distance " << distance << endl;
#endif
            unseen_starts.erase(current); 
        } else {
            if (visited.count(current)) {
                continue;
            } else {
                visited.insert(current);
            }
            
#ifdef debug_vg_algorithms
            cerr << "Visit " << g->get_id(current) << " " << g->get_is_reverse(current) << " at distance " << distance << endl;
#endif    

            // Emit the handle as being at the given distance
            if (!reached_callback(current, distance)) {
                // The user told us to stop.
                
                if (prune) {
                    // Just continue with whatever is next, and don't expand this
                    // node.
#ifdef debug_vg_algorithms
                    cerr << "\tPrune search" << endl;
#endif
                    stopped_early = true;
                    continue;
                } else {
                    // Stop right away.
                
                    // Return that we stopped early.
                
#ifdef debug_vg_algorithms
                    cerr << "\tAbort search" << endl;
#endif  
                
                    return false;
                }
            }
        }
        
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

    // Return whether we avoided needing to prune.
    return !stopped_early;

}

    
}
}
