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

# Usage Instructions


