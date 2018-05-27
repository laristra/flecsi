/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <bitset>
#include <tuple>

#include <cinchtest.h>
#include <flecsi/control/control.h>
#include <flecsi/control/phase_walker.h>

template<typename T, T M>
struct typeify {
  using TYPE = T;
  static constexpr T value = M;
};

template<size_t PHASE>
using phase_ = typeify<size_t, PHASE>;

enum simulation_phases_t : size_t {
  initialize,
  advance,
  analyze,
  io,
  mesh,
  finalize
}; // enum simulation_phases_t

enum target_attributes_t : size_t {
  time_advance_half = 0x01,
  time_advance_whole = 0x02,
  updated_eos_at_faces = 0x04
}; // enum target_attributes_t

using phases = std::tuple<
  phase_<initialize>,
  phase_<finalize>
>;

/*----------------------------------------------------------------------------*
 * Node policy
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

using namespace flecsi::control;

/*----------------------------------------------------------------------------*
 * Control policy.
 *----------------------------------------------------------------------------*/

using control_t = control__<node_policy_t>;
using phase_walker_t = phase_walker__<control_t>;

int action_a(int argc, char ** argv) {
  usleep(200000);
  std::cout << "target_a" << std::endl;
  return 0;
} // action_a

int action_b(int argc, char ** argv) {
  usleep(200000);
  std::cout << "target_b" << std::endl;
  return 0;
} // action_a

bool a_registered = control_t::instance().phase_map(initialize).
  initialize_node({ EXPAND_AND_STRINGIFY(a),
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(a)}.hash(),
  action_a, 0 });

bool b_registered = control_t::instance().phase_map(initialize).
  initialize_node({ EXPAND_AND_STRINGIFY(b),
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(b)}.hash(),
  action_b, 0 });

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

} // TEST
