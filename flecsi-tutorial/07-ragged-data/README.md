# FleCSI: Tutorial - 05 Ragged Data
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial::sparse-data) -->

# Ragged Data

The FleCSI ragged storage class is similar to the sparse storage class,
but without *column* indices, i.e., data are compacted into a compressed
storage layout, but no information is stored about the position of
a non-zero datum within the associated index space.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
