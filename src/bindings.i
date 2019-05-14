%module handlegraph

%{
#define SWIG_FILE_WITH_INIT
#include "handlegraph/handle_graph.hpp"
#include "handlegraph/path_handle_graph.hpp"
#include "handlegraph/mutable_handle_graph.hpp"
#include "handlegraph/deletable_handle_graph.hpp"
#include "handlegraph/mutable_path_handle_graph.hpp"
#include "handlegraph/mutable_path_mutable_handle_graph.hpp"
#include "handlegraph/mutable_path_deletable_handle_graph.hpp"
#include "handlegraph/empty_handle_graph.hpp"
// SWIG has no idea how to do namespaces itself
using namespace handlegraph;
%}

// Make sure SWIG knows about the C++ integer types
%include "stdint.i"

%include "handlegraph/types.hpp"
%include "handlegraph/handle_graph.hpp"
// Leave out path_for_each_socket.hpp because it only makes sense to use in C++.
// Its lack of default constructability also upsets SWIG.
// Also ignore the PathHandleGraph method that returns a PathForEachSocket.
%ignore PathHandleGraph;
%include "handlegraph/path_handle_graph.hpp"
%include "handlegraph/mutable_handle_graph.hpp"
%include "handlegraph/deletable_handle_graph.hpp"
%include "handlegraph/mutable_path_handle_graph.hpp"
%include "handlegraph/mutable_path_mutable_handle_graph.hpp"
%include "handlegraph/mutable_path_deletable_handle_graph.hpp"
%include "handlegraph/empty_handle_graph.hpp"




