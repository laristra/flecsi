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

#include <tuple>

#include <flecsi/control/control.h>
#include <flecsi/control/default_node.h>

namespace flecsi {
namespace control {

/*----------------------------------------------------------------------------*
 * Define simulation phases.
 *----------------------------------------------------------------------------*/

enum simulation_phases_t : size_t {
  initialize,
  driver
}; // enum simulation_phases_t

/*----------------------------------------------------------------------------*
 * Control policy.
 *----------------------------------------------------------------------------*/

struct unit_control_policy_t {

  using control_t = flecsi::control::control_u<unit_control_policy_t>;

  using node_t = flecsi::control::default_node_t;

  #define phase(name) flecsi::control::phase_<name>

  using phases = std::tuple<
    phase(initialize),
    phase(driver)
  >;

}; // struct unit_control_policy_t

using control_t =
  flecsi::control::control_u<flecsi::control::unit_control_policy_t>;

} // namespace control
} // namespace flecsi
