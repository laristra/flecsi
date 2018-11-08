#pragma once

#include <flecsi/utils/macros.h>
#include <flecsi/utils/const_string.h>

#include <io-poc/control/policy.h>

namespace io_poc {

using control_t = flecsi::control::control_u<io_poc::control_policy_t>;

} // namespace io_poc

#define hash_name(name)                                                        \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()

#define register_action(phase, name, action, ...)                              \
  bool name##_registered = io_poc::control_t::instance().phase_map(            \
    phase, EXPAND_AND_STRINGIFY(phase)).                                       \
    initialize_node({                                                          \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),        \
      EXPAND_AND_STRINGIFY(name),                                              \
      action, ##__VA_ARGS__ });

#define add_dependency(phase, to, from)                                        \
  bool registered_##to##from = io_poc::control_t::instance().phase_map(phase). \
    add_edge(flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(),   \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

#define check_attribute(phase, name, mask)                                     \
  io_poc::control_t::instance().phase_map(phase).node(hash_name(name)).        \
  bitset().to_ullong() & mask
