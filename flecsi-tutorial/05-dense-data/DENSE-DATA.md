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

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
