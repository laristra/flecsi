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
#include "flecsi/coloring/adjacency_types.h"
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
using namespace execution;
using namespace coloring;

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
    std::tuple<index_space_<0>, domain_<0>, cell>,
//    std::tuple<index_space_<2>, domain_<0>, edge>,
    std::tuple<index_space_<1>, domain_<0>, vertex>>;

  using connectivities = 
    std::tuple<std::tuple<index_space_<3>, domain_<0>, cell, vertex>>;

  using bindings = std::tuple<>;

  template<size_t M, size_t D, typename ST>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t<ST>* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            //return mesh->template make<edge>(*mesh);
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

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
data::legion::dense_handle_t<T, EP, SP, GP>;
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  data::mpi::dense_handle_t<T, EP, SP, GP>;
#endif

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void task1(client_handle_t<test_mesh_t, dro> mesh) {
  //np(y);
} // task1

void fill_task(client_handle_t<test_mesh_t, dwd> mesh) {
  clog(info) << "IN FILL TASK" << std::endl;

  auto & context = execution::context_t::instance();

  auto & vertex_map = context.index_map(1);
  auto & reverse_vertex_map = context.reverse_index_map(1);

  auto & cell_map = context.index_map(0);
  auto & reverse_cell_map = context.reverse_index_map(0);

  std::vector<vertex *> vertices;
  for(auto & vm: vertex_map) {
    vertices.push_back(mesh.make<vertex>());
  } // for

  clog(info) << "vertices: " << vertices.size() << std::endl;

  const size_t width = 8;

  size_t count(0);
  for(auto & cm: cell_map) {
    const size_t mid = cm.second;

    const size_t row = mid/width;
    const size_t column = mid%width;

    const size_t v0 = (column    ) + (row    ) * (width + 1);
    const size_t v1 = (column + 1) + (row    ) * (width + 1);
    const size_t v2 = (column + 1) + (row + 1) * (width + 1);
    const size_t v3 = (column    ) + (row + 1) * (width + 1);

    clog(info) << "mesh cell(" << count << "): (" << v0 << " " << v1 << " " << v2 << " " << v3 << ")" << std::endl;

    const size_t lv0 = reverse_vertex_map[v0];
    const size_t lv1 = reverse_vertex_map[v1];
    const size_t lv2 = reverse_vertex_map[v2];
    const size_t lv3 = reverse_vertex_map[v3];

    clog(info) << "local cell(" << count++ << "): (" << lv0 << " " << lv1 << " " << lv2 << " " << lv3 << ")" << std::endl;

    auto c = mesh.make<cell>();
    mesh.init_cell<0>(c,
      { vertices[lv0], vertices[lv1], vertices[lv2], vertices[lv3] });
  } // for

  mesh.init<0>();

  clog(info) << "MESH INIT" << std::endl;
} // fill_task

void print_task(client_handle_t<test_mesh_t, dro> mesh) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0){
    CINCH_CAPTURE() << "IN PRINT_TASK" << std::endl;

    for(auto c: mesh.entities<2, 0>()) {
      CINCH_CAPTURE() << "cell id: " << c->template id<0>() << std::endl;

      for(auto v: mesh.entities<0,0>(c)) {
        CINCH_CAPTURE() << "vertex id: " << v->template id<0>() << std::endl;
      } // for
    } // for

    ASSERT_TRUE(CINCH_EQUAL_BLESSED("data_client_handle.blessed"));
  }
} // print_task

void hello() {
  clog(info) << "Hello!!!" << std::endl;
} // hello

flecsi_register_data_client(test_mesh_t, meshes, mesh1); 

flecsi_register_task(task1, loc, single);

flecsi_register_field(test_mesh_t, hydro, pressure, double, dense, 1, 0);

flecsi_register_task(fill_task, loc, single);
flecsi_register_task(print_task, loc, single);
flecsi_register_task(hello, loc, single);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  flecsi_execute_mpi_task(add_colorings, 0);

  auto& context = execution::context_t::instance();

  auto& cc = context.coloring_info(0);
  auto& cv = context.coloring_info(1);

  adjacency_info_t ai;
  ai.index_space = 3;
  ai.from_index_space = 0;
  ai.to_index_space = 1;
  ai.color_sizes.resize(cc.size());

  for(auto& itr : cc){
    size_t color = itr.first;
    const coloring_info_t& ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  }

  context.add_adjacency(ai);
} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
//  auto& context = execution::context_t::instance();
//
//  auto runtime = Legion::Runtime::get_runtime();
//  const int my_color = runtime->find_local_MPI_rank();
//  clog(error) << "Rank " << my_color << " in specialization_spmd_init" << std::endl;

  {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);

  auto f1 = flecsi_execute_task(fill_task, single, ch);
  f1.wait();
  } // scope

  {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);

  auto f2 = flecsi_execute_task(print_task, single, ch);
  f2.wait();
  } // scope

} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
#if 0
  clog(info) << "In driver" << std::endl;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);

  flecsi_execute_task(task1, single, ch);
#endif
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
