/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <bitset>
#include <tuple>

#include <cinchtest.h>
#include <flecsi/control/control.h>
#include <flecsi/control/phase_walker.h>

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
 * Control policy (part of specialization).
 *----------------------------------------------------------------------------*/

using namespace flecsi::control;

struct control_policy_t {

  using control_t = control__<control_policy_t>;

  /*!
   */

  static bool evolve_control() {
    return control_t::instance().step()++ < 5;
  } // evolve

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

  size_t & step() { return step_; }

  struct node_t {

    using bitset_t = std::bitset<8>;
    using action_t = std::function<int(int, char **)>;

    node_t(action_t const & action = {}, bitset_t const & bitset = {})
      : action_(action), bitset_(bitset) {}

    bool initialize(node_t const & node) {
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

  }; // struct node_t

private:

  size_t step_;

}; // struct control_policy_t

std::ostream &
operator << (std::ostream & stream, control_policy_t::node_t const & node) {
  return stream;
} // operator <<

/*----------------------------------------------------------------------------*
 * Define control policy type.
 *----------------------------------------------------------------------------*/

using control_t = control__<control_policy_t>;

/*----------------------------------------------------------------------------*
 * Convenience
 *----------------------------------------------------------------------------*/

#if defined(FLECSI_ENABLE_GRAPHVIZ)
using graphviz_t = flecsi::utils::graphviz_t;
#endif

#define define_action(name)                                                    \
  int action_##name(int argc, char ** argv) {                                  \
    usleep(200000);                                                            \
    std::cout << "target_" << #name << std::endl;                              \
    return 0;                                                                  \
  }

define_action(init_mesh)
define_action(init_fields)
define_action(init_species)
define_action(advance_particles)
define_action(accumulate_currents)
define_action(update_fields)
define_action(poynting_flux)
define_action(restart_dump)
define_action(write_flux)
define_action(fixup_mesh)
define_action(finalize)
define_action(super_duper)

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

// Initialization
register_action(initialize, init_mesh, action_init_mesh);
register_action(initialize, init_fields, action_init_fields);
register_action(initialize, init_species, action_init_species);

// Advance
register_action(advance, advance_particles, action_advance_particles);
register_action(advance, accumulate_currents, action_accumulate_currents);
register_action(advance, update_fields, action_update_fields);

// Analysis
register_action(analyze, poynting_flux, action_poynting_flux);

// I/O
register_action(io, restart_dump, action_restart_dump);
register_action(io, write_flux, action_write_flux);

// Mesh
register_action(mesh, fixup_mesh, action_fixup_mesh);

#define ENABLE_M 0
#if ENABLE_M
register_action(advance, super_duper, action_super_duper);
#endif

add_dependency(initialize, init_fields, init_mesh);
add_dependency(initialize, init_species, init_mesh);

add_dependency(advance, accumulate_currents, advance_particles);
add_dependency(advance, update_fields, accumulate_currents);

#if ENABLE_M
add_dependency(advance, super_duper, advance_particles);
add_dependency(advance, accumulate_currents, super_duper);
#endif

register_action(finalize, finalize, action_finalize);

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

  auto & control = control_t::instance();
  control.execute(argc, &argv[0]);

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  graphviz_t gv;
  control.write(gv);
  gv.write("control.gv");
#endif

} // TEST
