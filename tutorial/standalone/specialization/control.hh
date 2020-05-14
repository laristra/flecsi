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

#include "flecsi/control.hh"
#include "flecsi/flog.hh"

namespace standalone {

enum class cp { initialize, advance, analyze, finalize };

inline const char *
operator*(cp control_point) {
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

  using control = flecsi::control<control_policy>;

  size_t & steps() {
    return steps_;
  }
  size_t & step() {
    return step_;
  }

  static bool cycle_control() {
    return control::instance().step()++ < 5;
  }

  template<auto CP>
  using control_point = flecsi::control_point<CP>;

  using cycle = flecsi::cycle<cycle_control,
    control_point<cp::advance>,
    control_point<cp::analyze>>;

  using control_points = std::
    tuple<control_point<cp::initialize>, cycle, control_point<cp::finalize>>;

private:
  size_t steps_{0};
  size_t step_{0};
}; // struct control_policy

using control = flecsi::control<control_policy>;

} // namespace standalone
