/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#include <array>

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/topology/set_topology.h>
#include <flecsi/topology/types.h>
#include <flecsi/coloring/coloring_types.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

using namespace std;
using namespace flecsi;
using namespace topology;
using namespace coloring;
using namespace execution;
using namespace supplemental;

class entity1 : public set_entity_t{
public:
  double x;
};

class set_types{
public:
  using entity_types = std::tuple<
    std::tuple<index_space_<0>, entity1>
    >;

  using independent_t = test_mesh_2d_t;

  size_t color(const entity1& e1, independent_t& dt){
    return 0;
  }

  size_t bin(const entity1& e1, independent_t& dt){
    return 0;
  }
};

using set_t = set_topology__<set_types>;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

flecsi_register_data_client(set_t, sets, set1); 

namespace flecsi {
namespace execution {

struct set_topology_index_space_t{
  size_t main_capacity;
  size_t active_migration_capacity;
};

struct set_topology_info_t{
  using index_space_map_t =
    std::unordered_map<size_t, set_topology_index_space_t>;

  index_space_map_t index_space_map;
};

void specialization_tlt_init(int argc, char ** argv) {
  auto & context = context_t::instance();

  context_t::set_topology_info_t info;

  context_t::set_topology_index_space_t is1;
  is1.main_capacity = 500;
  is1.active_migration_capacity = 20;

  info.index_space_map.emplace(0, std::move(is1));

  context.add_set_topology(info);
}

void specialization_spmd_init(int argc, char ** argv) {

}

void driver(int argc, char ** argv) {

}

} // namespace execution
} // namespace flecsi

TEST(set_topology, test1) {
  
} // TEST
