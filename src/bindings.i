%module handlegraph

%{
#define SWIG_FILE_WITH_INIT
#include "handlegraph/empty_handle_graph.hpp"
// SWIG has no idea how to do namespaces itself
using namespace handlegraph;
%}

// Make sure SWIG knows about the C++ integer types
%include "stdint.i"

%include "handlegraph/types.hpp"
%include "handlegraph/empty_handle_graph.hpp"




