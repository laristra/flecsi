/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#define DH20

#include <cinchtest.h>

#include "flecsi/execution/execution.h"
#include "flecsi/execution/context.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/coloring_types.h"
#include "flecsi/coloring/communicator.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/coloring/parmetis_colorer.h"
#include "flecsi/coloring/mpi_communicator.h"
#include "flecsi/topology/closure_utils.h"
#include "flecsi/utils/set_utils.h"
#include "flecsi/data/data.h"
#include "flecsi/data/data_client_handle.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/topology/mesh_topology.h"

using namespace std;
using namespace flecsi;
using namespace topology;

clog_register_tag(coloring);

class vertex : public mesh_entity_t<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
  vertex() = default;

};

class edge : public mesh_entity_t<1, 1>{
public:
};

class face : public mesh_entity_t<1, 1>{
public:
};

class cell : public mesh_entity_t<2, 1>{
public:

  using id_t = flecsi::utils::id_t;

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity<2> & c, id_t * e){
    id_t* v = c.get_entities(cell_id, 0);

    e[0] = v[0];
    e[1] = v[2];
    
    e[2] = v[1];
    e[3] = v[3];
    
    e[4] = v[0];
    e[5] = v[1];
    
    e[6] = v[2];
    e[7] = v[3];

    return {2, 2, 2, 2};
  }

};

class test_mesh_types_t{
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
    std::pair<domain_<0>, vertex>,
    std::pair<domain_<0>, edge>,
    std::pair<domain_<0>, cell>>;

  using connectivities = 
    std::tuple<std::tuple<domain_<0>, vertex, edge>,
               std::tuple<domain_<0>, vertex, cell>,
               std::tuple<domain_<0>, edge, vertex>,
               std::tuple<domain_<0>, edge, cell>,
               std::tuple<domain_<0>, cell, vertex>,
               std::tuple<domain_<0>, cell, edge>>;

  using bindings = std::tuple<>;

  template<size_t M, size_t D, typename ST>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t<ST>* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->template make<edge>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        }
        break;
      }
      default:
        assert(false && "invalid domain");
    }
  }
};

using test_mesh_t = mesh_topology_t<test_mesh_types_t>;

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = 
  data::legion::dense_handle_t<T, EP, SP, GP>;

template<typename DC>
using client_handle_t = data_client_handle__<DC>;

void task1(client_handle_t<test_mesh_t> mesh) {
  //np(y);
} // task1

flecsi_register_task(task1, loc, single);

class client_type : public flecsi::data::data_client_t{};

flecsi_register_data(client_type, ns, pressure, double, dense, 0, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;

  client_type c;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);

  flecsi_execute_task(task1, ch);
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(data_handle, testname) {
  
} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

#undef DH20
