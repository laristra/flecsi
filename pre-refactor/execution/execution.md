<!-- CINCHDOC
  DOCUMENT(developer-guide)
  SECTION(runtime-initialization-structure)
-->

# Runtime Initialization Structure

Internally, the FleCSI runtime goes through several initialization steps
that allow both the specialization and FleCSI types that are part of the
runtime to execute control point logic. The current structure of this
initialization is shown in Figure \ref{execution-structure}.

![FleCSI Runtime Initialization Structure. \label{execution-structure}](execution-structure.pdf)

<!-- CINCHDOC
  DOCUMENT(user-guide | developer-guide)
  SECTION(execution-model-overview)
-->

\newpage
# Execution Model Overview

FleCSI's execution model is hierarchical and data-centric, with work
being done through tasks and kernels:

* **task**<br>  
  Tasks operate on logically distributed address spaces with output
  determined only by inputs and with no observable side effects, i.e.,
  they are pure functions.

* **kernel**<br>  
  Kernels operate on logically shared memory and may have side
  effects within their local address space.

Both of these work types may be executed in parallel, so the attributes
that distinguish them from each other are best considered in the context
of memory consitency: Tasks work on locally-mapped data that are
logically distributed. Consistency of the distributed data state is
maintained by the FleCSI runtime. Each task instance always executes
within a single address space. Kernels work on logically shared data
with a relaxed consistency model, i.e., the user is responsible for
implementing memory consistency by applying synchronization techniques.
Kernels may only be invoked from within a task.

A simple, but incomplete view of this model is that tasks are internode,
and kernels are intranode, or, more precisely, intraprocessor. A more
complete view is much less restrictive of tasks, including only the
constraint that a task be a pure function.

Consider the following:

```cpp
double update(mesh<ro> m, field<rw, rw, ro> p) {
  double sum{0};

  forall(auto c: m.cells(owned)) {
    p(c) = 2.0*p(c);
    sum += p(c);
  } // forall

  return sum;
} // task
```

In this example, the *update* function is a task and the *forall* loop
implicitly defines a kernel. Ignoring many of the details, we see that
the task is *mostly* semantically sequential from a distributed-memory
point of view, i.e., it takes a mesh *m* and a field *p*, both of which
may be distributed, and applies updates as if the entire index space
were available locally. The admonishment that the task is *mostly*
semantically sequential refers to the *owned* identifier in the loop
construct. Here, *owned* indicates that the mesh should return an
iterator only to the exclusive and shared cell indices (see the
discussion on [index spaces](#index-spaces)). Implicitly, this exposes
the distributed-memory nature of the task.

The *forall* kernel, on the other hand, is explicitly data parallel. The
iterator *m.cells(owned)* defines an index space over which the loop
body may be executed in parallel. This is a very concise syntax that is
supported by the [FleCSI C++ language
extensions](#c++-language-extensions) (The use of the term
*kernel* in our nomenclature derives from the CUDA and OpenCL
programming models, which may be familiar to the reader.)

--------------------------------------------------------------------------------

<!-- CINCHDOC DOCUMENT(developer-guide) SECTION(execution-model) -->

# Execution Model

The FleCSI execution model is implemented through a combination of macro
and C++ interfaces. Macros are used to implement the high-level user
interface through calls to the core C++ interface. The general structure
of the code is illustrated in figure FIXME.

The primary interface classes context\_t, task\_model\_t, and
function\_t are types that are defined during configuration that encode
the low-level runtime that was selected for the build. These types are
then used in the macro definitions to implement the high-level FleCSI
interface. Currently, FleCSI supports the MPI and Legion
distributed-memory runtimes. The code to implement the backend
implementations for each of these is in the respectively named
sub-directory of *execution*, e.g., the Legion implementation is in
*legion*. Documentation for the macro and core C++ interfaces is
maintained in the Doxygen documentation. 

**Note:** Compile-time selection of the low-level runtime is handled
by the pre-processor through type definition in files of the form
*flecsi\_runtime\_X*, where *X* is a policy or runtime model, e.g.,
flecsi\_runtime\_context\_policy, or flecsi\_runtime\_execution\_policy.
These should be updated when new backend support is added or when a
runtime is removed. The files are located in the *flecsi* sub-directory.

## Tasks

FleCSI tasks are pure functions with controlled side-effects. This
variation of *pure* is required to allow runtime calls from within an
executing task. In general, data states are never altered by the
runtime, although they may be moved or managed. Any such changes
executed by the runtime will be transparent to the task and will not
alter correctness.

## Functions

The FleCSI function interface provides a mechanism for creating
relocatable function references in the form of a function handle.
Function handles are first-class objects that may be passed as data.
They are functionally equivalent to a C++
[std::function](http://en.cppreference.com/w/cpp/utility/functional/function).

## Kernels

FleCSI kernels are implicitly defined by data-parallel semantics. In
particular, the FleCSI C++ language extensions add the *forall* keyword.
Logically, each occurance of a forall loop defines a kernel that can be
applied to a given index space. This construct has relaxed memory
consistency and is similar to OpenCL or CUDA kernels, or to pragmatized
OpenMP loops.

<!-- CINCHDOC
  DOCUMENT(user-guide | developer-guide)
  SECTION(c++-language-extensions)
-->

# C++ Language Extensions

FleCSI provides fine-grained, data-parallel semantics through the
introduction of the several keywords: *forall*, *reduceall*, and *scan*.
These are extensions to standard C++ syntax. As an example, consider the
following task definition:
```
void update(mesh<ro> m, field<rw, rw, ro> p) {
  forall(auto c: m.cells(owned)) {
    p(c) += 1.0;
  } // forall
} // update
```
The forall loop defines a FleCSI *kernel* that can be executed in
parallel.

The descision to modify C++ is partially motivated by a desire to
capture parallel information in a direct way that will allow portable
optimizations. For example, on CPU architectures, the introduction of
loop-carried dependencies is often an important optimization step. A
pure C++ implementation of forall would inhibit this optimization
because the iterates of the loop would have to be executed as function
object invocations to comply with C++ syntax rules.  The use of a
language extension allows our compiler frontend to make optimization
decisions based on the target architecture, and gives greater
latitude on the code transformations that can be applied.

Another motivation is the added ability of our approach to integrate
loop-level parallelism with knowledge about task execution. Task
registration and execution in the FleCSI model explicitly specify on
which processor type the task shall be executed. This information can be
used in conjuntion with our parallel syntax to identify the target
architecture for which to optimize. Additionally, because tasks are pure
functions, any data motion required to reconcile depenencies between
separate address spaces can be handled during the task prologue and
epilogue stages. The Legion runtime also uses this information to hide
latency. This approach provides a much better option for making
choices about execution granulatiry that may affect data dependencies.
As an example of why this is useful, consider the contrasting example of
a direct C++ implementation of a forall interface:
```
```
In this case, we are effectively limited to expressing parallelism and
data movement at the level of the forall loop. In particular, if data
must be offloaded to an accelerator device to perform the loop body, the
overhead of that operation must be compensated for by the arithmetic
complexity of the loop logic for that single loop. Additionally, a
policy must also be specified as part of the signature of the interface.
This adds noise to the code that is avoidable.

Using a combination of FleCSI tasks and kernels allows the user to
separate the concerns of fine-grained data parallelism and
distributed-memory data dependencies:
```
// Task
//
// Mesh and field data will be resident in the correct
// address space by the time the task is invoked.
void update(mesh<ro> m, field<rw, rw, ro> p) {

  // Each of the following kernels can execute in parallel
  // if parallel execution is supported by the target architecture.

  // Kernal 1
  forall(auto c: m.cells(owned)) {
    p(c) += 1.0;
  } // forall

  // Kernal 2
  forall(auto c: m.cells(owned)) {
    p(c) *= 2.0;
  } // forall

  // Kernal 3
  forall(auto c: m.cells(owned)) {
    p(c) -= 1.0;
  } // forall

} // update
```
Tasks have low overhead, so if the user really desires loop-level
parallelization, they can still acheive this by creating a task with a
single forall loop. However, the distinction between tasks and kernels
provides a good mechanism for reasoning about data dependencies between
parallel operations.

# Mesh Coloring

FleCSI supports the partitioning or *coloring* of meshes through a
set of core interface methods that can be accessed by a
specialization. Using these methods, the specialization can generate
the independent entity coloring, and colorings for dependent entities,
e.g., a mesh that has a primary or *independent* coloring based on the
cell adjacency graph, with the *dependent* coloring of the vertices
generated from the cell coloring by applying a rule such as lowest color
ownership to assign the vertex owners.

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
