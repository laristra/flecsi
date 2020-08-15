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

/*! @file */

#ifndef __FLECSI_PRIVATE__
#define __FLECSI_PRIVATE__
#endif

#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/run/control.hh"
#include "flecsi/util/unit/types.hh"

#include <tuple>

namespace flecsi {
namespace unit {

/*!
  The test_control_points_enum type is part of the control
  specialization for FleCSI's unit test fraemwork. It provides indices
  for the available control points in a unit test.
 */

enum class test_control_points {
  initialization,
  driver,
  finalization
}; // enum test_control_points

inline const char * operator*(test_control_points cp) {
  switch(cp) {
    case test_control_points::initialization:
      return "initialization";
    case test_control_points::driver:
      return "driver";
    case test_control_points::finalization:
      return "finalization";
  }
  flog_fatal("invalid unit test control point");
}

/*!
  The control_policy type defines the control policy for
  the FleCSI unit test framework. It is a good example of a non-cycling
  control flow that provides basic control points.
 */

struct control_policy {

  using control_points_enum = test_control_points;

  struct node_policy {};

  template<auto CP>
  using control_point = flecsi::run::control_point<CP>;

  using control_points =
    std::tuple<control_point<control_points_enum::initialization>,
      control_point<control_points_enum::driver>,
      control_point<control_points_enum::finalization>>;

}; // struct control_policy

using control = flecsi::run::control<flecsi::unit::control_policy>;

template<control::target_type Target>
using initialization =
  control::action<Target, test_control_points::initialization>;

template<control::target_type Target>
using driver = control::action<Target, test_control_points::driver>;

template<control::target_type Target>
using finalization = control::action<Target, test_control_points::finalization>;

} // namespace unit
} // namespace flecsi
