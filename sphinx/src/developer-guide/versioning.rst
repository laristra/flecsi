.. |br| raw:: html

   <br />

Versioning
==========

Branche Types
*************

* **incompatible** |br|
  The *devel* branch is where work on the next major release takes
  place, potentially with interface and feature changes that are
  *incompatible* with previous versions.

* **feature** |br|
  Feature branches (named for their *major* version number, e.g., 1, 2,
  3) are for feature development on the current major version.

* **release** |br|
  Release branches (named for their *major.minor* version number, e.g.,
  1.1, 1.2) are stable versions of the code base. These can only be
  modified with bug fixes. At appropriate points, tags (named for their
  *major.minor.patch* version number, e.g., 1.1.2) are used to identify
  patched versions.

.. tip::

  At the time of writing, FleCSI has the following branches:

  **devel** (incompatible) |br|
  **1** (feature) |br|
  **1.4** (release)

Workflow
********

FleCSI development uses a *devel -> feature -> release* forking workflow
that can be visualized as in :numref:`branch`. Bugfixes and features can
be back-merged into *feature* or *devel*, as appropriate.

.. tip::

  Local development branches should use the nameing convention
  *devel-description*. In particular, FleCSI's version generation uses
  the *devel-* naming to recognize development branches.

.. _branch:
.. figure:: branch.png

  Workflow diagram illustrating basic workflow using the three branch
  types described above. (Figure due to Angela Herring.)

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
