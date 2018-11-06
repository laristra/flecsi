# FleCSI Backend Development

The FleCSI programming system provides a low-level interface to support
multiple backend runtimes. Current in-house developed support includes
Legion and MPI. This document provides information and requirements for
developers who wish to add new backend runtime implementations.

## Structure

Source code for the FleCSI library and runtime is located in the
*flecsi* directory of the FleCSI git repository (When you check out the
FleCSI repository, this will be the *flecsi* directory in the top-level
directory where the repository was cloned.) FleCSI defines control,
execution, and data models that must be implemented by each runtime
backend. The control and execution models are located in the *execution*
subdirectory, and the data model is located in the *data* directory.
FleCSI also provides some generic data structures for representing
various graph, tree, and set topologies. These are located in the
*topology* directory. Each of these subdirectories has runtime-specific
code that is located in a subdirectory named for the runtime, e.g., the
Legion-specific code for the execution and control models is located in
*flecsi/execution/legion*.

## Policies

FleCSI uses policy-based design to select different backend
implementations for a common interface. A complete discription due to
Andrei Alexandrescu is available
[here](https://www.amazon.com/Modern-Design-Generic-Programming-Patterns/dp/0201704315/ref=sr_1_1?ie=UTF8&qid=1519158334&sr=8-1&keywords=alexandrescu+c%2B%2B).
This design pattern is relatively simple, and has the form:

```cpp
template<typename POLICY>
struct interface_type_u : public POLICY
{
  decltype(auto) method(parameter_1 && p1, parameter_2 && p2, ...) {
    return POLICY::method(std::forward<parameter_1>(p1), ...);
  }
}; // struct
```

*Note that perfect forwarding should be used where possible.*

Policy-based design allows the interface to be split from the
implementation, and is a form of static polymorphism. The core FleCSI
interface is largely defined in this manner. This allows FleCSI to
present a common interface with multiple implementations, i.e., each
runtime implements a set of policies that are used to specialize the
FleCSI runtime and library interfaces. Adding support for a new runtime
is equivalent to implementing the required set of policies. FleCSI uses
simple preprocessing to define types using whichever backend is being
targeted. The preprocessor configuration files are located in the
*flecsi/runtime* directory. As an example, the FleCSI runtime context_t
type is configured in *flecsi/runtime/flecsi_runtime_context_policy.h*:

```cpp
// This code snippet shows type definition for the Legion runtime.

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/execution/legion/context_policy.h>

namespace flecsi {
namespace execution {

using FLECSI_RUNTIME_CONTEXT_POLICY = legion_context_policy_t;

} // namespace execution
} // namespace flecsi

#elif ...
```

This file is then included in the core interface in
*flecsi/execution/context.h*:

```cpp
// This code snippet shows how the prprocessor-defined type
// FLECSI_RUNTIME_CONTEXT_POLICY is used to specify the appropriate
// policy, depending on how FleCSI has been configured.

#include <flecsi/runtime/flecsi_runtime_context_policy.h>

namespace flecsi {
namespace execution {

/*!
  The context_t type is the high-level interface to the FleCSI runtime
  context.

  @ingroup execution
 */

using context_t = context_u<FLECSI_RUNTIME_CONTEXT_POLICY>;

} // namespace execution
} // namespace flecsi
```

The *context_t* type can then be used throughout the code and will
correctly use the specified backend.

The next several sections define and describe the various policies that
are required to implement the FleCSI interface.

### Context Policy

The context policy defines how FleCSI handles runtime meta-data and
interoperability. The runtime data primarily consist of
partitioning/coloring information, and index space mappings.
Runtime interoperability currently means interoperability with MPI,
e.g., the Legion runtime must perform a handshake with MPI and make sure
that all processes are synced before switching to MPI execution.

**Interface File:** *flecsi/execution/context.h*

**Legion Backend:** *flecsi/execution/legion/context_policy.{h,cc}*

**MPI Backend:** *flecsi/execution/mpi/context_policy.{h,cc}*

**Preprocessor Variable:** *FLECSI_RUNTIME_CONTEXT_POLICY*

### Storage Policy

The storage policy defines how FleCSI registers fields and data clients.
Data clients are described below in detail. Actual registration is
handled via a callback function that is executed during runtime
initialization.

**Interface File:** *flecsi/execution/storage.h*

**Legion Backend:** *flecsi/execution/legion/storage_policy.{h,cc}*

**MPI Backend:** *flecsi/execution/mpi/storage_policy.{h,cc}*

**Preprocessor Variable:** *FLECSI_RUNTIME_STORAGE_POLICY*

### Execution Policy

The execution policy defines how FleCSI tasks and functions are
registered, invoked, and executed.

**Interface File:** *flecsi/execution/task.h*, and
*flecsi/execution/funciton.h*

**Legion Backend:** *flecsi/execution/legion/execution_policy.h*

**MPI Backend:** *flecsi/execution/mpi/execution_policy.h*

**Preprocessor Variable:** *FLECSI_RUNTIME_EXECUTION_POLICY*

## Data Clients

From the point of view of the core FleCSI library, a data client is a
type that defines one or more index spaces on which data may be
registered. Examples of FleCSI data client types are: *mesh_topology_u*,
*tree_topology_u*, and *set_topology_u*. Each of these types provide
interfaces to iterators over the various entity types that make up the
elements of the topology. When a user registers a field, it is loosley
equivalent to adding a data member to the client type. At runtime, an
instance of a data client will have an instance of that field (data
member). FleCSI uses lazy allocation, so that fields are only allocated
for client instances that actually access that particular field. The
memory allocation for the field itself is handled through the FleCSI
data model. Since fields are registered against a particular index
space, the data client understands the memory requirements for the field
(since it defines the index space, and partitioning/coloring). The data
client uses this information to describe the data to the data model. It
is a client because it provides access to the data model *server*. All
state data are managed through the FleCSI data model.

*Note: Data clients themselves require data to represent their internal
state, e.g., agacency information, and entity instances. These data are
not excluded from the requirement that they use the data model, i.e.,
these data are also registered and allocated using the FleCSI data
model. This discussion is beyond the scope of this document. Please see
the FleCSI Developer Guide for more information.*

## Coloring

Need description of coloring types and interfaces.

<!-- Need explanation of translation unit requirements. -->

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
