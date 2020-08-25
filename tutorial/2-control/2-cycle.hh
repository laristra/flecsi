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

namespace cycle {

enum class cp { initialize, advance, analyze, finalize };

inline const char * operator*(cp control_point) {
  switch(control_point) {
    case cp::initialize:
      return "initialize";
    case cp::advance:
      return "advance";
    case cp::analyze:
      return "analyze";
    case cp::finalize:
      return "finalize";
  }
  flog_fatal("invalied control point");
}

struct control_policy {

  using control_points_enum = cp;
  struct node_policy {};

  using control = flecsi::run::control<control_policy>;

  /*
    Define a function to access the step_ data member.
   */

  size_t & step() {
    return step_;
  }

  /*
    The core FleCSI control model inherits from the control policy, so that
    any data members defined in your policy are carried with the control
    policy instance, and can be accessed through a static interface.
   */

  static bool cycle_control() {
    return control::instance().step()++ < 5;
  }

  template<auto CP>
  using control_point = flecsi::run::control_point<CP>;

  /*
    The cycle type. Cycles are similar to the control_points tuple, with the
    addition of a predicate function that controls termination of the cycle.
   */

  using cycle = flecsi::run::cycle<cycle_control,
    control_point<cp::advance>,
    control_point<cp::analyze>>;

  /*
    The control_points tuple type takes the cycle as one of its
    elements. Valid types for the control_points tuple are, therefore,
    either typeified enumeration values, or cycles.
   */

  using control_points = std::
    tuple<control_point<cp::initialize>, cycle, control_point<cp::finalize>>;

private:
  size_t step_{0};
};

using control = flecsi::run::control<control_policy>;

} // namespace cycle
