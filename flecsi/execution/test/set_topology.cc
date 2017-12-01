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

#include "flecsi/execution/execution.h"
#include "flecsi/topology/set_topology.h"
#include "flecsi/topology/types.h"
#include "flecsi/coloring/coloring_types.h"
#include "flecsi/data/data_client_handle.h"
#include "flecsi/data/data_handle.h"

using namespace std;
using namespace flecsi;
using namespace topology;
using namespace coloring;

class entity1 : public set_entity_t{

};

class entity2 : public set_entity_t{

};

class set_types{
public:
  using entity_types = std::tuple<
    std::tuple<index_space_<0>, entity1>
    >;
};

using set_t = set_topology_t<set_types>;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void task1(client_handle_t<set_t, rw> mesh) {

}

void add_set_colorings(int dummy) {
  execution::context_t & context_ = execution::context_t::instance();
  
  size_t n = 100;
  size_t colors = context_.colors();
  size_t nc = n/colors;

  std::unordered_map<size_t, coloring_info_t> coloring_info;
  
  for(size_t i = 0; i < colors; ++i){
    coloring_info_t ci;
    ci.exclusive = nc;
    ci.shared = 0;
    ci.ghost = 0;

    coloring_info[i] = std::move(ci);
  }

  context_.add_set_coloring(0, coloring_info);
}

flecsi_register_mpi_task(add_set_colorings);

flecsi_register_task(task1, loc, single);

flecsi_register_field(set_t, hydro, pressure, double, dense, 1, 0);

namespace flecsi {
namespace execution {

void specialization_tlt_init(int argc, char ** argv) {
  flecsi_execute_mpi_task(add_set_colorings, 0);
}

void specialization_spmd_init(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(set_t, sets, set1);
}

void driver(int argc, char ** argv) {

}

} // namespace execution
} // namespace flecsi

TEST(set_topology, test1) {
  
} // TEST