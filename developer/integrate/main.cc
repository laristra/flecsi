/*----------------------------------------------------------------------------*
 *----------------------------------------------------------------------------*/

#include <iostream>

#include <flecsi/utils/common.h>
#include <flecsi/utils/const_string.h>

#include <control.h>
#include <context.h>
#include <dag.h>
#include <specialization.h>

#include <unistd.h>

using namespace flecsi::utils;
using namespace flecsi::control;

#define define_action(name) \
  int action_##name(int argc, char ** argv) { \
    usleep(100000); \
    std::cout << "action_" << EXPAND_AND_STRINGIFY(name) << std::endl; \
  } // action

int space(int argc, char ** argv) {
  std::cout << std::endl << "**********************************" << std::endl;
} // space

define_action(advance_a)
define_action(advance_b)
define_action(advance_c)
define_action(advance_d)
define_action(advance_e)
define_action(advance_f)
define_action(advance_m)

define_action(initialize_a)
define_action(initialize_b)

define_action(finalize_a)
define_action(finalize_b)

#define flecsi_hash(name) \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()

#define flecsi_register_target(phase, name, callback, ...) \
  bool name##_registered = \
    flecsi::execution::context_t::instance().phase_map(phase). \
    update_node( \
      {flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(), \
      callback, ##__VA_ARGS__})

#define flecsi_add_dependency(phase, to, from) \
  bool registered_##to##from = \
    flecsi::execution::context_t::instance().phase_map(phase).add_edge( \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(), \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

flecsi_register_target(advance, b, action_advance_b,
  time_advance_half | updated_eos_at_faces);

flecsi_register_target(advance, c, action_advance_c);
flecsi_register_target(advance, e, action_advance_e);
flecsi_register_target(advance, f, action_advance_f);
flecsi_register_target(advance, space, space);

#define ENABLE_M 0

#if ENABLE_M
flecsi_register_target(advance, m, action_advance_m);
#endif

flecsi_add_dependency(advance, b, a);
flecsi_add_dependency(advance, c, a);
flecsi_add_dependency(advance, c, b);
flecsi_add_dependency(advance, e, d);
flecsi_add_dependency(advance, e, b);
flecsi_add_dependency(advance, e, f);
flecsi_add_dependency(advance, b, f);
flecsi_add_dependency(advance, space, e);

#if ENABLE_M
flecsi_add_dependency(advance, m, c);
flecsi_add_dependency(advance, d, m);
#endif

flecsi_register_target(advance, d, action_advance_d);
flecsi_register_target(advance, a, action_advance_a);

flecsi_add_dependency(advance, d, c);
flecsi_add_dependency(advance, d, a);

flecsi_register_target(initialize, ia, action_initialize_a);
flecsi_register_target(initialize, ib, action_initialize_b);
flecsi_register_target(initialize, ispace, space);

flecsi_register_target(finalize, fa, action_finalize_a);
flecsi_register_target(finalize, fb, action_finalize_b);

flecsi_add_dependency(initialize, ia, ib);
flecsi_add_dependency(initialize, ispace, ia);
flecsi_add_dependency(finalize, fa, fb);

int main(int argc, char ** argv) {

  std::bitset<8> bits(time_advance_whole | updated_eos_at_faces);
  std::cout << "bits: " << bits << std::endl;

  context_t::instance().init();

  control_t c;
  c.execute(argc, argv);

  return 0;
} // main
