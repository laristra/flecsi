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
#include <flecsi/data/global_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/execution/kernel.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <Kokkos_Core.hpp>

namespace flecsi {
namespace execution {

// ----------- data registration ----------------------------------------//
using test_mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle_u<DC, PS>;

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_field(test_mesh_t,
  hydro,
  pressure,
  double,
  dense,
  1,
  index_spaces::cells);

#if 0
flecsi_register_field(test_mesh_t,
  hydro,
  alpha,
  double,
  sparse,
  1,
  index_spaces::cells);
#endif

flecsi_register_global(global, int1, int, 1);
flecsi_register_color(color, int2, int, 1);

// ----------- tasks ----------------------------------------------------//

void
set_global_int(global_accessor_u<int, rw> global, int value) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();
  std::cout << "[" << rank << "] setting value" << std::endl;
  global = value;
}

flecsi_register_task_simple(set_global_int, loc, single);

void
init(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, rw, rw, na> pressure,
  color_accessor<int, rw> color) {

  color = 2;

  for(auto c : mesh.cells()) {
    pressure(c) = 0;
  }

  flecsi::parallel_for(
    mesh.cells(), KOKKOS_LAMBDA(auto c) { pressure(c) = 1.0; },
    std::string("init"));
}

flecsi_register_task_simple(init, loc, index);

void
test(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, ro, ro, ro> pressure,
  global_accessor_u<int, ro> global,
  color_accessor<int, ro> color) {

  flecsi::parallel_for(
    mesh.cells(),
    KOKKOS_LAMBDA(auto c) {
      ASSERT_EQ(pressure(c), 1.0);
      ASSERT_EQ(global, 2042);
      ASSERT_EQ(color, 2);
    },
    std::string("test"));

  forall(mesh.cells(), "test2") {
    ASSERT_EQ(pressure(i), 1.0);
    ASSERT_EQ(global, 2042);
    ASSERT_EQ(color, 2);
  }; // forall
}

flecsi_register_task_simple(test, loc, index);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {

  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();

  auto gh = flecsi_get_global(global, int1, int, 0);
  flecsi_execute_task_simple(set_global_int, single, gh, 2042);

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

  auto mh = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(mh, hydro, pressure, double, dense, 0);
  auto gh = flecsi_get_global(global, int1, int, 0);
  auto ch = flecsi_get_color(color, int2, int, 0);

  flecsi_execute_task_simple(init, index, mh, ph, ch);
  flecsi_execute_task_simple(test, index, mh, ph, gh, ch);
} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(simple_task, testname) {} // TEST

} // namespace execution
} // namespace flecsi
