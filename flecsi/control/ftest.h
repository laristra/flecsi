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

#define FLECSI_TEST(name)                                                      \
  CINCH_TEST(name)
#define FTEST(name)                                                            \
  CINCH_TEST(name)
#define FLECSI_CAPTURE()                                                       \
  CINCH_CAPTURE()
#define FLECSI_DUMP()                                                          \
  CINCH_DUMP()
#define FLECSI_EQUAL_BLESSED(f)                                                \
  CINCH_EQUAL_BLESSED(f)
#define FLECSI_WRITE(f)                                                        \
  CINCH_WRITE(f)

namespace flecsi {
namespace control {

enum simulation_phases_t : size_t {
  initialize,
  driver,
  finalize
}; // enum simulation_phases_t

struct ftest_control_policy_t {

  using control_t = flecsi::control::control_u<ftest_control_policy_t>;
  using node_t = flecsi::control::default_node_t;

  #define phase(name) flecsi::control::phase_<name>

  using phases = std::tuple<
    phase(initialize),
    phase(driver),
    phase(finalize)
  >;

}; // struct ftest_control_policy_t

using control_t =
  flecsi::control::control_u<flecsi::control::ftest_control_policy_t>;

} // namespace control
} // namespace flecsi

#if 0
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
#endif

#define ftest_register_initializer(action, ...)                                \
  bool initializer_##action##_registered =                                                     \
    flecsi::control::control_t::instance().phase_map(   \
      flecsi::control::initialize,                                               \
      EXPAND_AND_STRINGIFY(flecsi::control::initialize)).                        \
      initialize_node({                                                          \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(action)}.hash(),      \
        EXPAND_AND_STRINGIFY(action),                                            \
        action, ##__VA_ARGS__ });

#define ftest_add_initializer_dependency(to, from)                             \
  bool registered_initializer_##to##from =                                                 \
    flecsi::control::control_t::instance().phase_map(flecsi::control::initialize).              \
      add_edge(flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(), \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

#define ftest_register_driver(action, ...)                                     \
  bool driver_##action##_registered = flecsi::control::control_t::instance().phase_map(            \
    flecsi::control::driver, EXPAND_AND_STRINGIFY(flecsi::control::driver)).   \
    initialize_node({                                                          \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(action)}.hash(),      \
      EXPAND_AND_STRINGIFY(action),                                            \
      action, ##__VA_ARGS__ });

#define ftest_add_driver_dependency(to, from)                                  \
  bool registered_driver_##to##from =                                                 \
    flecsi::control::control_t::instance().phase_map(flecsi::control::driver).                  \
      add_edge(flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(), \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

#include <flecsi/execution/context.h>

inline bool unit_tla_registered =
  flecsi::execution::context_t::instance().register_top_level_action(
    flecsi::control::control_t::execute);
