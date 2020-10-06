.. |br| raw:: html

   <br />

.. _TUT-CM:

Execution Model
***************

FleCSI has two mechanisms for expressing work:

* **Tasks** |br|
  Tasks operate on data distributed to one or more address spaces, and
  use data privileges to maintain memory consistency. FleCSI tasks are
  like a more flexible version of MPI that does not require the user to
  explicitly update dependencies between different ranks, and which does
  not use static process mappings, i.e., relocatable, distributed-memory
  data parallelism.

* **Kernels** |br|
  Kernels operate on data in a single address space, but require
  explicit barriers to ensure consistency. This is generally referred to
  as a relaxed-consistency memory model. The kernel interface in
  FleCSI is defined by three parallel operations: *forall*, *reduceall*,
  and *scan*. Each of these is a fine-grained, data-parallel operation.
  The use of the *kernel* nomenclature is derived from CUDA, and OpenCL,
  and is conceptually consistent with those models.

----

Tasks
*****

Example 1: Single Tasks
+++++++++++++++++++++++

Example 2: Index Tasks
++++++++++++++++++++++

Launch Domains
^^^^^^^^^^^^^^

Example 3: MPI Tasks
++++++++++++++++++++

Kernels
*******

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
