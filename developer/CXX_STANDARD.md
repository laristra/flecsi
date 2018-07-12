![logo](../doc/flecsi.png)

# FleCSI C++ Standards Usage

This document is intended to capture design patterns and specific code
instances in FleCSI that depend on certain C++ language featuers that
are only available after a particular standard, i.e., c++14, c++17, or
c++2x. The goals that we are trying to achieve by documenting these
features are to serve as requirements for ASC procurements, and in order
to access our level of risk.

This document will need to be updated  as C++ features are added, and as
existing featues mature and are ubiquitously available in vendor
compilers.

# Current State

* C++11 is low-risk
* C++17 is medium to high risk
* C++2x has not yet been accepted by the committee

# C++17 Language Features (Used by FleCSI)

* **inline variables**<br>
  Inline variables are used in all of the object-factory-like
  registration data structures in FleCSI. This feature is useful because
  it allows factory registration from header files. Previously, this
  would result in multiple symbol definitions.

* **if constexpr**<br>
  Using *if constexpr* statements allows us to remove most, if not all,
  instances of SFINAE. The reulting code is more readable and more
  clearly expresses intent.

# C++17 Library Features (Used by FleCSI)

* **std::optional**<br>
  This type allows more versatile state implementations.

* **std::any**<br>
  This type allows us to remove our own implementation.

* **std::invoke**<br>
  This library call allows us to remove our own implementation.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
