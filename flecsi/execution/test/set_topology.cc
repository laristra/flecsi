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

flecsi_register_data_client(set_t, sets, set1); 

flecsi_register_task(task1, loc, single);

flecsi_register_field(set_t, hydro, pressure, double, dense, 1, 0);

namespace flecsi {
namespace execution {

void specialization_tlt_init(int argc, char ** argv) {

}

void specialization_spmd_init(int argc, char ** argv) {

}

void driver(int argc, char ** argv) {
  //auto ch = flecsi_get_client_handle(set_t, sets, set1);
}

} // namespace execution
} // namespace flecsi

TEST(set_topology, test1) {
  
} // TEST
