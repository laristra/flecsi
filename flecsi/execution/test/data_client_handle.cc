/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

// FIXME what's this?
#define DH20

#include <cinchtest.h>

#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

clog_register_tag(coloring);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using test_mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void
task1(client_handle_t<test_mesh_t, ro> mesh) {
  // np(y);
} // task1

void
fill_task(
    client_handle_t<test_mesh_t, wo> mesh,
    dense_accessor<double, rw, rw, ro> pressure) {
  size_t count = 0;
  for (auto c : mesh.cells()) {
    pressure(c) = count++;
  } // for
} // fill_task

void
print_task(
    client_handle_t<test_mesh_t, ro> mesh,
    dense_accessor<double, ro, ro, ro> pressure) {
  CINCH_CAPTURE() << "IN PRINT_TASK" << std::endl;

  for (auto c : mesh.entities<2, 0>()) {
    CINCH_CAPTURE() << "cell id: " << c->template id<0>() << std::endl;

    for (auto v : mesh.entities<0, 0>(c)) {
      CINCH_CAPTURE() << "vertex id: " << v->template id<0>() << std::endl;
    } // for

    clog(info) << "presure: " << pressure(c) << std::endl;
  } // for

} // print_task

void
hello() {
  clog(info) << "Hello!!!" << std::endl;
} // hello

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_task_simple(task1, loc, single);

flecsi_register_field(
    test_mesh_t,
    hydro,
    pressure,
    double,
    dense,
    1,
    index_spaces::cells);

flecsi_register_task_simple(fill_task, loc, single);
flecsi_register_task_simple(print_task, loc, single);
flecsi_register_task_simple(hello, loc, single);

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
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, single, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, dense, 0);

  flecsi_execute_task_simple(fill_task, single, ch, ph);
  auto future = flecsi_execute_task_simple(print_task, single, ch, ph);
  future.wait(); // wait before comparing results

  auto & context = execution::context_t::instance();
  if (context.color() == 0) {
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("data_client_handle.blessed"));
  }
} // scope

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(data_handle, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

#undef DH20
