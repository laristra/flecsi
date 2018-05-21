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
    usleep(1000000); \
    std::cout << "action_" << EXPAND_AND_STRINGIFY(name) << std::endl; \
  } // action

define_action(a)
define_action(b)
define_action(c)
define_action(d)
define_action(e)
define_action(f)

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

flecsi_register_target(advance, b, action_b, 0 | 1 | 2);

flecsi_register_target(advance, c, action_c);
flecsi_register_target(advance, e, action_e);
flecsi_register_target(advance, f, action_f);

flecsi_add_dependency(advance, b, a);
flecsi_add_dependency(advance, c, a);
flecsi_add_dependency(advance, c, b);
flecsi_add_dependency(advance, e, d);
flecsi_add_dependency(advance, e, b);
flecsi_add_dependency(advance, e, f);
flecsi_add_dependency(advance, b, f);

flecsi_register_target(advance, d, action_d);
flecsi_register_target(advance, a, action_a);

flecsi_add_dependency(advance, d, c);
flecsi_add_dependency(advance, d, a);

int main(int argc, char ** argv) {

#if 0
  size_t a = flecsi_hash(a);
  size_t b = flecsi_hash(b);
  size_t c = flecsi_hash(c);
  size_t d = flecsi_hash(d);
  size_t e = flecsi_hash(e);
  size_t f = flecsi_hash(f);

  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "c: " << c << std::endl;
  std::cout << "d: " << d << std::endl;
  std::cout << "e: " << e << std::endl;
  std::cout << "f: " << f << std::endl;

  dag_t g;

  g.add_edge(b,a);
  g.add_edge(c,a);
  g.add_edge(c,b);
  g.add_edge(d,c);
  g.add_edge(d,a);
  g.add_edge(e,d);
  g.add_edge(e,b);
  g.add_edge(e,f);
  g.add_edge(b,f);

  g.node(a).action() = action_a;
  g.node(b).action() = action_b;
  g.node(c).action() = action_c;
  g.node(d).action() = action_d;
  g.node(e).action() = action_e;
  g.node(f).action() = action_f;

  auto sorted = g.sort();

  auto sorted =
    flecsi::execution::context_t::instance().phase_map(0).sort();

  for(auto n: sorted) {
    n(argc, argv);
  } // for

#endif

  context_t::instance().init();

  control_t c;
  c.execute(argc, argv);

  return 0;
} // main
