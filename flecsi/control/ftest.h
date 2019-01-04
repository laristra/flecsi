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

#include <tuple>

#include <flecsi/control/control.h>
#include <flecsi/control/default_node.h>
#include <flecsi/utils/flog.h>
#include <flecsi/utils/macros.h>

#include <cinch/ctest.h>

namespace flecsi {
namespace control {

enum simulation_phases_t : size_t {
  initialize,
  driver
}; // enum simulation_phases_t

struct ftest_control_policy_t {

  using control_t = flecsi::control::control_u<ftest_control_policy_t>;
  using node_t = flecsi::control::default_node_t;

  #define phase(name) flecsi::control::phase_<name>

  using phases = std::tuple<
    phase(initialize),
    phase(driver)
  >;

}; // struct ftest_control_policy_t

using control_t =
  flecsi::control::control_u<flecsi::control::ftest_control_policy_t>;

} // namespace control
} // namespace flecsi

int unit_initialization(int argc, char ** argv);
int unit_driver(int argc, char ** argv);

inline bool unit_initialization_registered =
  flecsi::control::control_t::instance().phase_map(
    flecsi::control::initialize, "initialize").initialize_node({
      flecsi::utils::const_string_t{"initialize"}.hash(),
      "unit_initialization", unit_initialization
    });

inline bool unit_driver_registered =
  flecsi::control::control_t::instance().phase_map(
    flecsi::control::driver, "driver").initialize_node({
      flecsi::utils::const_string_t{"driver"}.hash(),
      "unit_driver", unit_driver
    });

#include <flecsi/execution/context.h>

inline bool unit_tla_registered =
  flecsi::execution::context_t::instance().register_top_level_action(
    flecsi::control::control_t::execute);
