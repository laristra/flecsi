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

enum cp {
  initialization,
  advance,
  advance_subcycle,
  analyze,
  io,
  mesh,
  finalization
};

inline const char * operator*(cp control_point) {
  switch(control_point) {
    case initialization:
      return "initialize";
    case advance:
      return "advance";
    case advance_subcycle:
      return "advance_subcycle";
    case analyze:
      return "analyze";
    case io:
      return "io";
    case mesh:
      return "mesh";
    case finalization:
      return "finalize";
  }
  flog_fatal("invalid control point");
}

inline int step{0};
inline int substep{0};

struct control_policy {

  using control_points_enum = cp;

  struct node_policy {};

  template<auto CP>
  using control_point = flecsi::run::control_point<CP>;

  static bool subcycle_control() {
    if(substep % 3 < 2) {
      flog(info) << "substep: " << substep % 3 << std::endl;
    } // if
    return substep++ % 3 < 2;
  }

  using subcycle = flecsi::run::cycle<subcycle_control,
    control_point<control_points_enum::advance_subcycle>>;

  static bool cycle_control() {
    if(step++ < 2) {
      flog(info) << "step: " << step << std::endl;
      return true;
    }
    return false;
  }

  using cycle = flecsi::run::cycle<cycle_control,
    control_point<control_points_enum::advance>,
    subcycle,
    control_point<control_points_enum::analyze>,
    control_point<control_points_enum::io>,
    control_point<control_points_enum::mesh>>;

  using control_points =
    std::tuple<control_point<control_points_enum::initialization>,
      cycle,
      control_point<control_points_enum::finalization>>;
};

using control = flecsi::run::control<control_policy>;
