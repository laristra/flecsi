/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/coloring/parmetis_colorer.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/data/mutator_handle.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/mutator.h>

using namespace std;
using namespace flecsi;
using namespace topology;
using namespace execution;
using namespace coloring;

clog_register_tag(coloring);

class vertex : public mesh_entity__<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
  vertex() = default;

};

class edge : public mesh_entity__<1, 1>{
public:
};

class face : public mesh_entity__<1, 1>{
public:
};

class cell : public mesh_entity__<2, 1>{
public:

  using id_t = flecsi::utils::id_t;

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity__<2> & c, id_t * e){
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

}; // class cell

class test_mesh_types_t{
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 1;

  using id_t = flecsi::utils::id_t;

  using entity_types = std::tuple<
    std::tuple<index_space_<0>, domain_<0>, cell>,
//    std::tuple<index_space_<2>, domain_<0>, edge>,
    std::tuple<index_space_<1>, domain_<0>, vertex>>;

  using connectivities = 
    std::tuple<std::tuple<index_space_<3>, domain_<0>, cell, vertex>>;

  using bindings = std::tuple<>;

  template<size_t M, size_t D, typename ST>
  static mesh_entity_base__<num_domains>*
  create_entity(mesh_topology_base__<ST>* mesh, size_t num_vertices,
    id_t const & id){
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

struct test_mesh_t : public mesh_topology__<test_mesh_types_t> {};

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void task1(client_handle_t<test_mesh_t, ro> mesh, sparse_mutator<double> mh) {

  auto& context = execution::context_t::instance();
  auto rank = context.color();
  auto coloring_info = context.coloring_info(mh.h_.index_space).at(rank);

  for(size_t i = 0; i < coloring_info.exclusive + coloring_info.shared; ++i){
    for(size_t j = 0; j < 5; j+=2){
      mh(i, j) = i * 100 + j + rank * 10000;
    }
  }
  //mh.dump();
} // task1

void task2(client_handle_t<test_mesh_t, ro> mesh,
           sparse_accessor<double, ro, ro, ro> h) {
  auto& context = execution::context_t::instance();
  auto rank = context.color();
  if (rank == 0){
    for (auto index : h.indices()) {
      for (auto entry : h.entries(index)) {
        CINCH_CAPTURE() << index << ":" << entry << ": " << h(index, entry) << 
          std::endl;
      }
    }
  }

} // task2

void task3(client_handle_t<test_mesh_t, ro> mesh,
           sparse_accessor<double, rw, rw, rw> h) {

  auto& context = execution::context_t::instance();
  auto rank = context.color();
  auto coloring_info = context.coloring_info(h.handle.index_space).at(rank);

  for (auto index : h.indices()) {
    for (auto entry : h.entries(index)) {
      h(index, entry) = -h(index, entry);
    }
  }

} // task3

void task4(client_handle_t<test_mesh_t, ro> mesh, sparse_mutator<double> mh) {

  auto& context = execution::context_t::instance();
  auto rank = context.color();
  auto coloring_info = context.coloring_info(mh.h_.index_space).at(rank);

  for(size_t i = 0; i < coloring_info.exclusive; ++i){
    for(size_t j = 5; j < 7; ++j){
      mh(i, j) = i * 100 + j + rank * 10000;
    }
  }
  //mh.dump();
} // task4
flecsi_register_data_client(test_mesh_t, meshes, mesh1); 

flecsi_register_task_simple(task1, loc, single);
flecsi_register_task_simple(task2, loc, single);
flecsi_register_task_simple(task3, loc, single);
flecsi_register_task_simple(task4, loc, single);

flecsi_register_field(test_mesh_t, hydro, pressure, double, sparse, 1, 0);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;
  flecsi_execute_mpi_task(add_colorings, flecsi::execution, map);

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

  execution::context_t::sparse_index_space_info_t isi;
  isi.max_entries_per_index = 5;
  isi.exclusive_reserve = 8192;
  isi.max_exclusive_entries = 8192;
  context.set_sparse_index_space_info(0, isi);
} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {

} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto mh = flecsi_get_mutator(ch, hydro, pressure, double, sparse, 0, 5);

  auto f0 = flecsi_execute_task_simple(task1, single, ch, mh);
  f0.wait();

  auto ph = flecsi_get_handle(ch, hydro, pressure, double, sparse, 0);

  auto f1 = flecsi_execute_task_simple(task2, single, ch, ph);
  f1.wait();

  auto f2 = flecsi_execute_task_simple(task3, single, ch, ph);
  f2.wait();

  auto f3 = flecsi_execute_task_simple(task2, single, ch, ph);
  f3.wait();

  auto& context = execution::context_t::instance();
  if(context.color() == 0){
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("sparse_data.blessed"));
  }

} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(sparse_data, testname) {
  
} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
