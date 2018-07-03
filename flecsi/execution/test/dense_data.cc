/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2018, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

///
/// \file
/// \date Initial file creation: Apr 3, 2018
///

#include <cinchtest.h>

#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<size_t PS>
using mesh = data_client_handle__<mesh_t, PS>;

template<size_t EP, size_t SP, size_t GP>
using field = dense_accessor<size_t, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, meshes, mesh1);
flecsi_register_field(
    mesh_t,
    hydro,
    pressure,
    size_t,
    dense,
    1,
    index_spaces::cells);

//----------------------------------------------------------------------------//
// Init field
//----------------------------------------------------------------------------//

void
init(mesh<ro> mesh, field<rw, rw, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(owned)) {
    size_t val = c->index()[1] + 100 * (c->index()[0] + 100 * rank);
    h(c) = 1000000000 + val * 100;
  }

  for (auto c : mesh.cells(shared)) {
    h(c) += 5;
  }
} // init

flecsi_register_task(init, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Print field
//----------------------------------------------------------------------------//

void
print(mesh<ro> mesh, field<ro, ro, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(exclusive)) {
    CINCH_CAPTURE() << "[" << rank << "]: exclusive cell id " << c->id<0>()
                    << "(" << c->index()[0] << "," << c->index()[1]
                    << "): " << h(c) << std::endl;
  }

  for (auto c : mesh.cells(shared)) {
    CINCH_CAPTURE() << "[" << rank << "]:    shared cell id " << c->id<0>()
                    << "(" << c->index()[0] << "," << c->index()[1]
                    << "): " << h(c) << std::endl;
  }

  for (auto c : mesh.cells(ghost)) {
    CINCH_CAPTURE() << "[" << rank << "]:     ghost cell id " << c->id<0>()
                    << "(" << c->index()[0] << "," << c->index()[1]
                    << "): " << h(c) << std::endl;
  }
} // print

flecsi_register_task(print, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Modify field
//----------------------------------------------------------------------------//

void
modify(mesh<ro> mesh, field<rw, rw, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(owned)) {
    h(c) += 10 * (rank + 1);
  }

} // modify

flecsi_register_task(modify, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, single, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, size_t, dense, 0);

  flecsi_execute_task(init, flecsi::execution, single, ch, ph);
  flecsi_execute_task(print, flecsi::execution, single, ch, ph);

  flecsi_execute_task(modify, flecsi::execution, single, ch, ph);
  auto future = flecsi_execute_task(print, flecsi::execution, single, ch, ph);
  future.wait(); // wait before comparing results

  auto & context = execution::context_t::instance();
  if (context.color() == 0) {
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("dense_data.blessed"));
  }

} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(dense_data, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
