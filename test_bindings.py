#!/usr/bin/env python3

import handlegraph

graph = handlegraph.EmptyHandleGraph()

print("Node size: ", graph.node_size())
assert(graph.node_size() == 0)
print("Max ID: ", graph.max_node_id())
assert(graph.max_node_id() == 0)
assert(graph.has_node(123) == False)

assert(isinstance(graph, handlegraph.HandleGraph))

count = 0
def handle_hendle(handle):
    count += 1
graph.for_each_handle(handle_handle)
assert(count == 0)

print("Python bindings are working correctly")
