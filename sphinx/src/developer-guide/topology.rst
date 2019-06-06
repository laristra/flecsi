.. |br| raw:: html

   <br />

Topology Requirements
=====================

To develop a new topology type from an existing type, e.g., mesh,
octree, or kd-tree, the user should follow these steps:

* Split Interface & Storage

* Define a topology handle type

* Specialize topology registration

* Specialize storage class implementations


Topology Initialization Workflow
================================

1. User defines specialization policy

2. User defines topology type with policy

3. Register meta data fields for specialized topology type

4. User adds fields to topology-defined index spaces

5. User gets topology instance

6. User generates coloring and calls set_coloring on instance

7. FleCSI creates index spaces and index partitions

8. FleCSI invokes task to initialize topology meta data

9. User invokes task to initialize field state

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
