%module handlegraph

%{
#define SWIG_FILE_WITH_INIT
#include "handlegraph/empty_handle_graph.hpp"
// SWIG has no idea how to do namespaces itself
using namespace handlegraph;
%}

%include "handlegraph/empty_handle_graph.hpp"



