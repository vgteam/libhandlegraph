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

%{
// Define a class to wrap Python Callables on the C++ side.
// Definition taken from <https://stackoverflow.com/a/11522655>.
// Really serves as a reference-counting handle to an underlying Python object
// that we have by pointer, so when you get passed a pointer you need to
// construct a new one of these.
//
// We use SWIG-internal functions so definitions have to go at the end.
class PyCallback
{
    PyObject *func;
    PyCallback& operator=(const PyCallback&); // Not allowed
public:
    PyCallback(const PyCallback& o);
    PyCallback(PyObject *func);
    ~PyCallback();
    
    // Take a call coming in from C++ and pass it to Python
    bool operator()(const handle_t& s) const;
};
%}

// Now add support to the HandleGraph for looping over PyObjects with PyCallback
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

// TODO: we should be able to just do this. But SWIG throws runtime errors
// about self being an argument when it only wants two arguments.
//%extend handlegraph::HandleGraph {
//  %template(for_each_handle_callback) for_each_handle<PyCallback>;
//}

// So extend with a manual implementation instead
%extend handlegraph::HandleGraph {
  bool for_each_handle(PyObject *callback) {
    return $self->for_each_handle(PyCallback(callback));
  }
  
  bool follow_edges(const handle_t& source, bool left, PyObject *callback) {
    return $self->follow_edges(source, left, PyCallback(callback));
  }
}


%{
// Now PyCallback definitions, after SWIG defines its functions

PyCallback::PyCallback(const PyCallback& o) : func(o.func) {
  Py_XINCREF(func);
}
PyCallback::PyCallback(PyObject *func) : func(func) {
  Py_XINCREF(this->func);
  assert(PyCallable_Check(this->func));
}
PyCallback::~PyCallback() {
  Py_XDECREF(func);
}

bool PyCallback::operator()(const handle_t& s) const {
  if (!func || Py_None == func || !PyCallable_Check(func))
    return false;
    
  // Create the Python arguments
  PyObject* handle_arg = SWIG_NewPointerObj((new handlegraph::handle_t(s)), SWIGTYPE_p_handlegraph__handle_t, SWIG_POINTER_OWN |  0 );
  // Pack into a tuple  
  PyObject *args = Py_BuildValue("(O)", handle_arg);
  
  // Make the Python call and get the result
  PyObject *result = PyObject_Call(func,args,0);
  
  // Convert result to bool
  bool converted_result;
  int ecode = SWIG_AsVal_bool(result, &converted_result);
  if (!SWIG_IsOK(ecode)) {
      // Just treat it as true (keep going)
      converted_result = true;
  } 
  
  Py_DECREF(result);
  Py_DECREF(args);
  Py_XDECREF(handle_arg);
  
  return converted_result;
}

%}





