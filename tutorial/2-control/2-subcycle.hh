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

enum class cp { initialize, advance, advance2, analyze, finalize };

inline const char * operator*(cp control_point) {
  switch(control_point) {
    case cp::initialize:
      return "initialize";
    case cp::advance:
      return "advance";
    case cp::advance2:
      return "advance2";
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
    Define a function to access the substep_ data member;
   */

  size_t & substep() {
    return substep_;
  }

  size_t & step() {
    return step_;
  }

  /*
    Define a subcycle control function.
   */

  static bool subcycle_control() {
    return control::instance().substep()++ % 3 < 2;
  }

  static bool cycle_control() {
    return control::instance().step()++ < 5;
  }

  template<auto CP>
  using control_point = flecsi::run::control_point<CP>;

  /*
    Define a subcycle type.
   */

  using subcycle = flecsi::run::cycle<subcycle_control,
    control_point<cp::advance>,
    control_point<cp::advance2>>;

  using cycle =
    flecsi::run::cycle<cycle_control, subcycle, control_point<cp::analyze>>;

  using control_points = std::
    tuple<control_point<cp::initialize>, cycle, control_point<cp::finalize>>;

private:
  size_t substep_{0};
  size_t step_{0};
};

using control = flecsi::run::control<control_policy>;

} // namespace cycle
