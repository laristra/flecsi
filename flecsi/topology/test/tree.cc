#include <cinchtest.h>
#include <cmath>
#include <iostream>

#include <flecsi/data/data_client_handle.h>
#include <flecsi/execution/execution.h>
#include <flecsi/topology/tree/tree_topology.h>
#include <flecsi/topology/types.h>

#include "pseudo_random.h"
#include <flecsi/topology/tree/tree_topology.h>

using namespace std;
using namespace flecsi;
using namespace topology;

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

class tree_policy
{
public:
  // Basic elements
  static constexpr size_t dimension = 3;
  using element_t = double;
  using point_t = flecsi::point_u<element_t, dimension>;
  using filling_curve_int_t = uint64_t;
  // The space filling curve used
  using filling_curve_t =
    flecsi::hilbert_curve_u<dimension, filling_curve_int_t>;
  // The tree topology itself
  using tree_entity_ =
    flecsi::topology::tree_entity_u<dimension, filling_curve_t>;
  using tree_entity_holder_ =
    flecsi::topology::tree_entity_holder_u<dimension, filling_curve_t>;
  using tree_branch_ = flecsi::topology::
    tree_branch_u<dimension, tree_entity_holder_, filling_curve_t>;

  using entity_types = std::tuple<std::tuple<index_space_<0>, tree_entity_>,
    std::tuple<index_space_<1>, tree_entity_holder_>,
    std::tuple<index_space_<2>, tree_branch_>>;
}; // class tree_policy

using tree_topology_t = topology::tree_topology_u<tree_policy>;
using entity_t = tree_topology_t::entity_t;
using tree_entity_t = tree_topology_t::tree_entity_t;

using point_t = tree_topology_t::point_t;
static constexpr size_t dimension = tree_topology_t::dimension;
// using branch_t = tree_topology_u::branch_t;
// using branch_id_t = tree_topology_u::branch_id_t;
using element_t = tree_topology_t::element_t;
using range_t = tree_topology_t::range_t;

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

pseudo_random rnd;

TEST(tree_topology, generation) {
  auto t = new tree_topology_t();
  size_t num_entities = 64;

  std::vector<entity_t> entities(num_entities);
  std::vector<tree_entity_t> tree_entities(num_entities);
  std::vector<utils::id_t> entities_id(num_entities);
  std::vector<utils::id_t> tree_entities_id(num_entities);

  auto storage = t->storage();
  storage->init_entities(&(entities[0]), &(entities_id[0]), &(tree_entities[0]),
    &(tree_entities_id[0]), num_entities);

  range_t range = {point_t{1, 1, 1}, point_t{0, 0, 0}};

  // 1. Generate the entities and store them in the tree
  for(size_t i = 0; i < num_entities; ++i) {
    point_t p = {rnd.uniform(), rnd.uniform(), rnd.uniform()};
    auto e = t->make_entity(p);
    for(int d = 0; d < dimension; ++d) {
      range[0][d] = std::min(range[0][d], p[d]);
      range[1][d] = std::max(range[1][d], p[d]);
    }
  }
  t->set_range(range);

  std::cout << *t << std::endl;

  // 2. Generate the keys for each entities
  t->generate_keys();

  // 3. Sort the entities
  t->sort_entities();

  // 4. Make the tree == assert particles sorted and key generated
  t->build_tree();

  // 5. Compute the COFM information, starting from the root
  t->cofm();

  // 5. Output the current tree
  t->graphviz(0);

  delete t;

} // TEST
