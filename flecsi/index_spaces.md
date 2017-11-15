<!-- CINCHDOC DOCUMENT(developer-guide) SECTION(index-spaces) -->

# Index Spaces

The FleCSI data and execution models are premised on the notion of index
spaces.  Simply put, an index space is an enumerated set. An index space
can be defined in several ways. For example, an index space with which
many people are familiar is a range. Consider the loop

```cpp
    for(int i=0; i<5; ++i) { std::cout << "i=" << i << std::endl; }
```

Implicitly, the values that are taken on by *i* form an index space, i.e.,
the set of enumerated indices

```
    { 0, 1, 2, 3, 4 }
```

A more relevant example is the set of indices for the canonical triangle
mesh in Figure \ref{canonical-mesh}. Both the cells and the vertices
represent index spaces. The cells index space has 29 objects, indexed
from 1 to 29

```
    { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 },
```

while the vertex index space has 23 objects, indexed from 1 to 23

```
    { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23 }.
```

![A canonical triangle mesh example with cells and vertices.\label{canonical-mesh}](canonical-mesh.pdf)

A basic understanding of index spaces and these  examples is critical to
understanding the FleCSI data model.

**Formally, a space is a set of indices with some added structure. An
enumerated set satisfies this definition. However, the enumerated sets
in FleCSI are better classified as topological spaces.  In general,
FleCSI index spaces--discussed here and below--represent discrete
topologies. Colloquilly, index spaces may also be referred to as index
sets.**

## Subsets

Given an index space, we can form subsets that only contain some of the
objects of the original set, e.g.,

```
    { 4, 9, 10, 11, 16, 17, 18, 19, 24, 28 }
```

is a proper subset of the cells index space in Figure
\ref{canonical-mesh}

```
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
      17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 }
```

Subsets are a useful way to define distributed-memory colorsings.
Consider the disjoint partitioning of our canonical mesh in Figure
\ref{colored-mesh}. We can represent this coloring by defining the
index spaces

```
    1. { 4, 9, 10, 11, 16, 17, 18, 19, 24, 28 }
    2. { 1, 2, 5, 6, 12, 13, 20, 25, 29 }
    3. { 3, 7, 8, 14, 15, 21, 22, 23, 26, 27 }
```

![Disjoint coloring of our canonical triangle mesh.\label{colored-mesh}](colored-mesh.pdf)

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
