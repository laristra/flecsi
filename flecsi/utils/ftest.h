/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#ifndef __FLECSI_PRIVATE__
#define __FLECSI_PRIVATE__
#endif

#include <flecsi/control/control.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/flog.h>
#include <flecsi/utils/ftest/node.h>
#include <flecsi/utils/ftest/types.h>

#include <tuple>

namespace flecsi {
namespace control {

enum simulation_control_points_t : size_t {
  initialize,
  test,
  finalize
}; // enum simulation_control_points_t

struct ftest_control_policy_t {

  using control_t = flecsi::control::control_u<ftest_control_policy_t>;
  using node_t = flecsi::utils::ftest::node_t;

#define control_point(name) flecsi::control::control_point_<name>

  using control_points = std::tuple<control_point(initialize), control_point(test), control_point(finalize)>;

}; // struct ftest_control_policy_t

using control_t =
  flecsi::control::control_u<flecsi::control::ftest_control_policy_t>;

/*
  Register a command-line option "--control-model" to output a dot file
  that can be used to visualize the control points and actions of an
  ftest executable. This macro can be used for any qualified control_u
  specialization (not just the ftest example here).
 */

flecsi_register_control_options(control_t);

} // namespace control
} // namespace flecsi

#define ftest_register_initialize(action, ...)                                 \
  inline bool ftest_initialize_##action##_registered =                         \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::initialize,                                  \
        EXPAND_AND_STRINGIFY(flecsi::control::initialize))                     \
      .initialize_node(                                                        \
        {flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(action)}.hash(),   \
          EXPAND_AND_STRINGIFY(action), action, ##__VA_ARGS__});

#define ftest_add_initialize_dependency(to, from)                              \
  inline bool ftest_registered_initialize_##to##from =                         \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::initialize)                                  \
      .add_edge(                                                               \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(),        \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

#define ftest_register_test(action, ...)                                       \
  inline bool ftest_test_##action##_registered =                               \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(                                                              \
        flecsi::control::test, EXPAND_AND_STRINGIFY(flecsi::control::test))    \
      .initialize_node(                                                        \
        {flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(action)}.hash(),   \
          EXPAND_AND_STRINGIFY(action), action, ##__VA_ARGS__});

#define ftest_add_test_dependency(to, from)                                    \
  inline bool ftest_registered_test_##to##from =                               \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::test)                                        \
      .add_edge(                                                               \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(),        \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

#define ftest_register_finalize(action, ...)                                   \
  inline bool ftest_finalize_##action##_registered =                           \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::finalize,                                    \
        EXPAND_AND_STRINGIFY(flecsi::control::finalize))                       \
      .initialize_node(                                                        \
        {flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(action)}.hash(),   \
          EXPAND_AND_STRINGIFY(action), action, ##__VA_ARGS__});

#define ftest_add_finalize_dependency(to, from)                                \
  inline bool ftest_registered_finalize_##to##from =                           \
    flecsi::control::control_t::instance()                                     \
      .control_point_map(flecsi::control::finalize)                                    \
      .add_edge(                                                               \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(),        \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

#include <flecsi/execution/context.h>

inline bool unit_tla_registered =
  flecsi::execution::context_t::instance().register_top_level_action(
    flecsi::control::control_t::execute);
