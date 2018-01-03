# FleCSI: Tutorial Specialization
<!--
  The above header ("FleCSI: Tutorial") is required for Doxygen to
  correctly name the auto-generated page. It is ignored in the FleCSI
  guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial) -->

# Tutorial Specialization

FleCSI is intended as a developer tool that can be used to create
interfaces for application developers and scientists. As such, FleCSI
requires a *specialization* layer that customizes the core FleCSI data
structures to provide an interface that makes sense for the particular
application or set of applications that are being targeted. In this
sense, FleCSI is a tool for creating domain-specific interfaces.

The source code in this directory defines a specialization that supports
the interfaces used in our tutorial. In particular, this tutorial
specialization defines a two-dimensional unstructured mesh interface
that is suitable for applications requiring nearest-neighbor ghost
dependencies.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
