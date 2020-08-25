/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "flecsi/flog.hh"
#include "flecsi/run/control.hh"

namespace simple {

/*
  Enumeration defining the control point identifiers. This will be used to
  specialize the core control type.
 */

enum class cp { initialize, advance, finalize };

/*
  Define labels for the control points. Here we use function call operator
  overloading of the '*' operator. This approach is safer than defining a satic
  array of string literals because it is type-safe, and it allows error
  checking.
 */

inline const char * operator*(cp control_point) {
  switch(control_point) {
    case cp::initialize:
      return "initialize";
    case cp::advance:
      return "advance";
    case cp::finalize:
      return "finalize";
  }
  flog_fatal("invalied control point");
}

/*
  Control policy for this example. The control policy primarily defines types
  that are used by the core control type to define the control-flow model for
  the program. For this simple example, the policy captures the user-defined
  enumeration of control-points identifiers, defines an empty node policy, and
  defines the order of control point execution using std::tuple.
 */

struct control_policy {

  /*
    Capture the control points enumeration type. This type is used in the
    control policy interface whenever a control point is required.
   */

  using control_points_enum = cp;

  /*
    The actions that are added under each control point are used to form a
    directed acyclic graph (DAG). The node_policy type allows the user to add
    interfaces and data that are available from, and are carried with the
    action. A more complex example will demonstrate this capability.
   */

  struct node_policy {};

  /*
    This is a convenience type definition that provides typeification of an
    integer value.
   */

  template<auto CP>
  using control_point = flecsi::run::control_point<CP>;

  /*
    The control_points tuple defines the actual control points as typeified
    integers derived from the control point identifiers from the user-defined
    enumeration.
   */

  using control_points = std::tuple<control_point<cp::initialize>,
    control_point<cp::advance>,
    control_point<cp::finalize>>;
};

/*
  Define a fully-qualified control type for the end user.
 */

using control = flecsi::run::control<control_policy>;

} // namespace simple
