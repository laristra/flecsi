# FleCSI: Tutorial - 05 Dense Data
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial::dense-data) -->

# Dense Data

The FleCSI data model provides several different storage types. A
storage type is a formal term that implies a particular logical layout
to the data of types registered under it. The "logical" layout of the
data provides the user with an interface that is consistent with a
particular view of the data. In this example, we focus on the "dense"
storage type.

Logically, dense data may be represented as a contiguous array, with
indexed access to its elements. The dense storage type interface is
similar to a C array or std::vector, with element access provided
through the "()" operator, i.e., if var has been registered as a dense
data type, it may be accessed like: var(0), var(1), etc.

In the fields example, we were actually using the dense storage type to
represent an array of double defined on the cells of our specialization
mesh. This example is an extension that demonstrates using user-defined
types as dense field data.  

This example uses a user-defined struct as the dense data type. The
*struct_type_t* is defined in the "types.h" file:

```cpp
#pragma once

#include <flecsi/data/dense_accessor.h>

namespace types {

using namespace flecsi;

// This is the definition of the struct_type_t type.

struct struct_type_t {
  double a;
  size_t b;
  double v[3];
}; // struct_type_t

// Here, we define an accessor type to use in the task signatures in our
// example. Notice that the base type that we are using "dense_accessor"
// takes four parameters. The first parameter is the type that has been
// registered on the associated index space. The other three parameters
// specify the privileges with which the corresponding data will be
// accessed.

template<
  size_t SHARED_PRIVILEGES>
using struct_field = dense_accessor<struct_type_t, rw, SHARED_PRIVILEGES, ro>;

} // namespace types
```

Aside from using a struct type, this example of registering data is
identical to registering a fundamental type, e.g., double or size_t.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
