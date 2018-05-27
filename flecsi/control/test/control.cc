/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <bitset>
#include <tuple>

#include <cinchtest.h>
#include <flecsi/control/control.h>
#include <flecsi/control/phase_walker.h>

// FIXME: typeify needs to move into common.h in utils.
template<typename T, T M>
struct typeify {
  using TYPE = T;
  static constexpr T value = M;
};

template<size_t PHASE>
using phase_ = typeify<size_t, PHASE>;

/*----------------------------------------------------------------------------*
 * Define simulation phases. This is considered part of the specializeation.
 *----------------------------------------------------------------------------*/

enum simulation_phases_t : size_t {
  initialize,
  advance,
  analyze,
  io,
  mesh,
  finalize
}; // enum simulation_phases_t

/*----------------------------------------------------------------------------*
 * Define action attributes. This is considered part of the specialization.
 *----------------------------------------------------------------------------*/

enum action_attributes_t : size_t {
  time_advance_half = 0x01,
  time_advance_whole = 0x02,
  updated_eos_at_faces = 0x04
}; // enum action_attributes_t

/*----------------------------------------------------------------------------*
 * Node policy (part of specialization).
 *----------------------------------------------------------------------------*/

struct node_policy_t {

  using bitset_t = std::bitset<8>;
  using action_t = std::function<int(int, char **)>;

  node_policy_t(action_t const & action = {}, bitset_t const & bitset = {})
    : action_(action), bitset_(bitset) {}

  bool initialize(node_policy_t const & node) {
    action_ = node.action_;
    bitset_ = node.bitset_;
    return true;
  } // initialize

  action_t const & action() const { return action_; }
  action_t & action() { return action_; }

  bitset_t const & bitset() const { return bitset_; }
  bitset_t & bitset() { return bitset_; }

private:

  action_t action_;
  bitset_t bitset_;

}; // struct node_policy_t

inline std::ostream &
operator << (std::ostream & stream, node_policy_t const & node) {
  stream << "action: " << &node.action() << std::endl;
  stream << "bitset: " << node.bitset() << std::endl;
  return stream;
} // operator <<

/*----------------------------------------------------------------------------*
 * Control policy.
 *----------------------------------------------------------------------------*/

using namespace flecsi::control;

using control_t = control__<node_policy_t>;
using phase_walker_t = phase_walker__<control_t>;
using phase_writer_t = phase_writer__<control_t>;

#if defined(FLECSI_ENABLE_GRAPHVIZ)
using graphviz_t = flecsi::utils::graphviz_t;
#endif

size_t step{0};

// Fixme: This needs to go into the control context somehow.
bool evolve_control() {
  return step++ < 1;
} // evolve_control

/*----------------------------------------------------------------------------*
 * Define the phases and sub cycles.
 *----------------------------------------------------------------------------*/

using evolve = cycle__<
  evolve_control, // stopping predicate
  phase_<advance>,
  phase_<analyze>,
  phase_<io>,
  phase_<mesh>
>;

using phases = std::tuple<
  phase_<initialize>,
  evolve,
  phase_<finalize>
>;

/*----------------------------------------------------------------------------*
 * Convenience
 *----------------------------------------------------------------------------*/

#define define_action(name) 																    \
  int action_##name(int argc, char ** argv) { 											 \
    usleep(200000); 																			    \
    std::cout << "target_" << #name << std::endl; 								       \
	 return 0; 																						 \
  }

define_action(a)
define_action(b)
define_action(c)
define_action(d)
define_action(e)
define_action(f)
define_action(g)
define_action(h)
define_action(p)
define_action(m)

#define register_action(phase, name, action)                                   \
  bool name##_registered = control_t::instance().phase_map(                    \
    phase, EXPAND_AND_STRINGIFY(phase)).                                       \
    initialize_node({                                                          \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),        \
      EXPAND_AND_STRINGIFY(name),                                              \
      action, 0 });

#define add_dependency(phase, to, from)                                        \
  bool registered_##to##from = control_t::instance().phase_map(phase).         \
    add_edge(flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(),   \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())

/*----------------------------------------------------------------------------*
 * These define the control point actions and DAG dependencies.
 *----------------------------------------------------------------------------*/

register_action(initialize, a, action_a);

register_action(advance, b, action_b);
register_action(advance, c, action_c);
register_action(advance, d, action_d);
register_action(advance, e, action_e);

register_action(io, g, action_g);
register_action(mesh, h, action_h);

register_action(analyze, p, action_p);

#define M 1
#if M
register_action(advance, m, action_m);
#endif

add_dependency(advance, c, b);
add_dependency(advance, b, d);

#if M
add_dependency(advance, m, d);
add_dependency(advance, e, m);
#endif

add_dependency(advance, e, d);

register_action(finalize, f, action_f);

/*----------------------------------------------------------------------------*
 * Run the test...
 *----------------------------------------------------------------------------*/

TEST(control, testname) {

  // fake command-line arguments
  int argc = 1;
  char ** argv;
  argv = new char *[1];
  argv[0] = new char[1024];
  strcpy(argv[0], "control");

  control_t::instance().init();

  phase_walker_t phase_walker(argc, &argv[0]);
  phase_walker.template walk_types<phases>();

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  graphviz_t gv;

  phase_writer_t phase_writer(gv);
  phase_writer.template walk_types<phases>();

  gv.write("control.gv");
#endif

} // TEST
