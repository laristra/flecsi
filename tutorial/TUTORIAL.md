# FleCSI Tutorial

## Requirements

This tutorial assumes that you have successfully downloaded and built
FleCSI, and that you have installed it somewhere on your system.

## FleCSI Users

The structure of applications built on top of the FleCSI programming
system assumes three basic types of users. Each of the user types has
their own set of responsibilities that are designed to separate
concerns, and to make sure that development tasks are intuitive and
acheivable by the associated user type.

The user types are:

* **FleCSI Core Developer**  
These are users who design, implement, and maintain the core FleCSI
library. Generally, these users are expert C++ developers who have a
well-developed understanding of the the low-level design of the FleCSI
software architecture. These users are generally computer scientists
with expertise in generic programming techniques, data structure design,
and optimization.

* **FleCSI Specialization Developer**  
These are users who adapt the core FleCSI data structures and runtime
interfaces to create domain-specific interfaces for application
developers.  These users are required to understand the components of
the FleCSI interface that can be statically specialized, and must have a
solid understanding of the runtime interface. Additionally,
specialization develoeprs are assumed to understand the requirements of
the application area for which they are designing an interface. These
users are generally computational scientists with expertise in one or
more numerical methods areas.

* **FleCSI Application Developer**
These users are methods developers or physiciests who use a particular
FleCSI specialization layer to develop and maintain application codes.
These are the FleCSI end-users, who have expertise in designing and
implementing numerical methods to solve complicated, multiphysics
simulation problems.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
