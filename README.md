# `libhandlegraph`: A handle-based abstraction for graph access

This library defines a set of interfaces for "Handle Graphs": genome variation
graphs where all access to the graphs is mediated by opaque "handle" objects.

It is primarily used in [vg](https://github.com/vgteam/vg) as an abstraction
layer over different backing graph implementations.

# Build Instructions

```
mkdir build
cd build
cmake ..
make
```

To install to the default prefix (`/lib`):

```
make install
```

Or to another prefix:

```
make DESTDIR=/another/prefix install
```

# Usage Instructions

There are headers corresponding to the different handle graph interface types:

```
#include <handlegraph/handle_graph.hpp>
#include <handlegraph/mutable_handle_graph.hpp>
#include <handlegraph/mutable_path_deletable_handle_graph.hpp>
...
```

The classes live in the `handlegraph` namespace.

To link against the library (which contains the default implementations of methods and operators on handles), use `-lhandlegraph`.



