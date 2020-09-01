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

#include <vector>

namespace state {

enum class cp { allocate, initialize, advance, finalize };

inline const char * operator*(cp control_point) {
  switch(control_point) {
    case cp::allocate:
      return "allocate";
    case cp::initialize:
      return "initialize";
    case cp::advance:
      return "advance";
    case cp::finalize:
      return "finalize";
  }
  flog_fatal("invalied control point");
}

struct control_policy {

  using control_points_enum = cp;
  struct node_policy {};

  using control = flecsi::run::control<control_policy>;

  static bool cycle_control() {
    return control::instance().step()++ < control::instance().steps();
  }

  template<auto CP>
  using control_point = flecsi::run::control_point<CP>;

  using cycle = flecsi::run::cycle<cycle_control, control_point<cp::advance>>;

  using control_points = std::tuple<control_point<cp::allocate>,
    control_point<cp::initialize>,
    cycle,
    control_point<cp::finalize>>;

  /*--------------------------------------------------------------------------*
    State interface
   *--------------------------------------------------------------------------*/

  size_t & step() {
    return step_;
  }

  size_t & steps() {
    return steps_;
  }

  void allocate_values(size_t size) {
    values_ = new size_t[size];
  }

  void deallocate_values() {
    delete[] values_;
  }

  size_t * values() const {
    return values_;
  }

private:
  /*--------------------------------------------------------------------------*
    State members
   *--------------------------------------------------------------------------*/

  size_t step_{0};
  size_t steps_{0};
  size_t * values_;
};

using control = flecsi::run::control<control_policy>;

} // namespace state
