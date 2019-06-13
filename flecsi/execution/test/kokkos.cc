/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include <cinchlog.h>
#include <cinchtest.h>

#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/execution/kernel.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <Kokkos_Core.hpp>

namespace flecsi {
namespace execution {

using test_mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle_u<DC, PS>;

void
task1(client_handle_t<test_mesh_t, ro> mesh,
		dense_accessor<double, rw, rw, na> pressure) {

  for (auto c:mesh.cells()){
   pressure(c)=0;
  }

  auto comp = [=](auto c){
    pressure(c) = 1.0;
  };

  const std::string task_name = "task1";
  flecsi::parallel_for(mesh.cells(), comp, task_name);

}

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_task_simple(task1, loc, index);

flecsi_register_field(test_mesh_t,
  hydro,
  pressure,
  double,
  dense,
  1,
  index_spaces::cells);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();
} // specialization_tlt_init

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  clog(info) << "Inside user driver" << std::endl;

  Kokkos::print_configuration(std::cerr);

  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, dense, 0);

  flecsi_execute_task_simple(task1, index, ch, ph);
} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(simple_task, testname) {} // TEST

} // namespace execution
} // namespace flecsi
