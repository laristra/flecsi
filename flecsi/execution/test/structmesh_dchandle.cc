/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#define DCH_SMESH

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/coloring/simple_box_colorer.h>
#include <flecsi/supplemental/coloring/add_box_colorings.h>
#include <flecsi/data/mutator_handle.h>
#include <flecsi/data/dense_accessor.h>

using namespace flecsi;
using namespace topology;
using namespace execution;
using namespace coloring;

clog_register_tag(coloring);

class vertex : public structured_mesh_entity__<0, 1>{
public:
}; //class vertex

class edge : public structured_mesh_entity__<1, 1>{
public:
}; //class edge

class cell : public structured_mesh_entity__<2, 1>{
public:
}; //class cell

class test_mesh_type_t{
public:
  static constexpr size_t num_dimensions = 2;
  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
  std::tuple<index_space_<0>, domain_<0>, vertex>,
  std::tuple<index_space_<1>, domain_<0>, edge>,
  std::tuple<index_space_<2>, domain_<0>, cell>>;
};

struct test_mesh_t : public mesh_topology__<test_mesh_type_t> {};

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void task1(client_handle_t<test_mesh_t, ro> mesh) {
  //np(y);
} // task1

void fill_task(client_handle_t<test_mesh_t, wo> mesh,
  dense_accessor<double, rw, rw, ro> pressure) {
  clog(info) << "IN FILL TASK" << std::endl;

  count = 0;
  for(auto c: mesh.entities<2,0>()) {
    pressure(c) = count++;
  } // for
} // fill_task

void print_task(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, ro, ro, ro> pressure) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0){
    CINCH_CAPTURE() << "IN PRINT_TASK" << std::endl;

    for(auto c: mesh.entities<2, 0>()) {
      CINCH_CAPTURE() << "cell id: " << c->template id<0>() << std::endl;
      clog(info) << "presure: " << pressure(c) << std::endl;
    } // for

//    ASSERT_TRUE(CINCH_EQUAL_BLESSED("data_client_handle.blessed"));
  }
} // print_task

void hello() {
  clog(info) << "Hello!!!" << std::endl;
} // hello

flecsi_register_data_client(test_mesh_t, meshes, mesh1); 
flecsi_register_field(test_mesh_t, hydro, pressure, double, dense, 1, 0);

flecsi_register_task_simple(task1, loc, single);
flecsi_register_task_simple(fill_task, loc, single);
flecsi_register_task_simple(print_task, loc, single);
flecsi_register_task_simple(hello, loc, single);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  coloring_map_t map;
  map.cells = 0;
  flecsi_execute_mpi_task(add_box_colorings, flecsi::execution, map);

  auto& context = execution::context_t::instance();

} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
//  auto& context = execution::context_t::instance();
//
//  auto runtime = Legion::Runtime::get_runtime();
//  const int my_color = runtime->find_local_MPI_rank();
//  clog(error) << "Rank " << my_color << " in specialization_spmd_init" << std::endl;

  {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, dense, 0);

  auto f1 = flecsi_execute_task_simple(fill_task, single, ch, ph);
  f1.wait();
  } // scope

  {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, dense, 0);

  auto f2 = flecsi_execute_task_simple(print_task, single, ch, ph);
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

  flecsi_execute_task_simple(task1, single, ch);
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

#undef DCH_SMESH
