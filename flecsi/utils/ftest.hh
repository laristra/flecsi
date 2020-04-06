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

#include "../control.hh"
#include "../execution.hh"
#include <flecsi/flog.hh>
#include <flecsi/utils/const_string.hh>
#include <flecsi/utils/ftest/node.hh>
#include <flecsi/utils/ftest/types.hh>

#include <tuple>

namespace flecsi {
namespace control {

/*!
  The simulation_control_points_t type is part of the control
  specialization for FleCSI's unit test fraemwork. It provides labels
  for the available control points in a unit test.
 */

enum simulation_control_points_t : size_t {
  initialize,
  driver,
  finalize
}; // enum simulation_control_points_t

/*!
  The ftest_control_policy_t type defines the control policy for
  the FleCSI unit test framework. It is a good example of a non-cycling
  control flow that provides basic control points.
 */

struct ftest_control_policy_t {

  using control_t = flecsi::control::control<ftest_control_policy_t>;
  using node_t = flecsi::utils::ftest::node_t;

#define control_point(name) flecsi::control::point_<name>

  using control_points = std::tuple<control_point(initialize),
    control_point(driver),
    control_point(finalize)>;

#undef control_point

}; // struct ftest_control_policy_t

using control_t =
  flecsi::control::control<flecsi::control::ftest_control_policy_t>;

/*
  Register a command-line option "--control-model" to output a dot file
  that can be used to visualize the control points and actions of an
  ftest executable. This macro can be used for any qualified control
  specialization (not just the ftest example here).
 */

flecsi_register_control_options(control_t);

} // namespace control
} // namespace flecsi

#define ftest_register_action(action, control_point, ...)                      \
  inline bool ftest_initialize_##action##_registered =                         \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::control_point,                       \
        flecsi_internal_stringify(flecsi::control::control_point))             \
      .initialize_node({flecsi_internal_hash(action),                          \
        flecsi_internal_stringify(action),                                     \
        action,                                                                \
        ##__VA_ARGS__})

#define ftest_add_dependency(control_point, to, from)                          \
  inline bool ftest_registered_initialize_##to##from =                         \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::control_point)                       \
      .add_edge(flecsi_internal_hash(to), flecsi_internal_hash(from))

#define ftest_register_initialize(action, ...)                                 \
  ftest_register_action(action, initialize, ##__VA_ARGS__)

#define ftest_add_initialize_dependency(to, from)                              \
  ftest_add_dependency(initialize, to, from)

#define ftest_register_driver(action, ...)                                     \
  ftest_register_action(action, driver, ##__VA_ARGS__)

#define ftest_add_driver_dependency(to, from)                                  \
  ftest_add_dependency(driver, to, from)

#define ftest_register_finalize(action, ...)                                   \
  ftest_register_action(action, finalize, ##__VA_ARGS__)

#define ftest_add_finalize_dependency(to, from)                                \
  ftest_add_dependency(finalize, to, from)
