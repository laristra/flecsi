/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#define DH50

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

clog_register_tag(coloring);

namespace flecsi {
namespace execution {

using test_mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void
task1(client_handle_t<test_mesh_t, ro> mesh, ragged_mutator<double> rm) {
  rm.resize(1, 5);

  rm(1, 0) = 100.0;
  rm(1, 1) = 200.0;
  rm(1, 4) = 500.0;
  rm.push_back(1, 700.00);

} // task1

void
task1b(client_handle_t<test_mesh_t, ro> mesh, ragged_mutator<double> rm) {
  rm.insert(1, 4, 300.0);
  rm.erase(1, 1);
} // task1

void
task2(
    client_handle_t<test_mesh_t, ro> mesh,
    ragged_accessor<double, ro, ro, ro> rh) {

  for (size_t i = 0; i < 6; ++i) {
    std::cout << rh(1, i) << std::endl;
  }

} // task2

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_task_simple(task1, loc, single);
flecsi_register_task_simple(task1b, loc, single);
flecsi_register_task_simple(task2, loc, single);

flecsi_register_field(
    test_mesh_t,
    hydro,
    pressure,
    double,
    ragged,
    1,
    index_spaces::cells);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();

  context_t::sparse_index_space_info_t isi;
  isi.index_space = index_spaces::cells;
  isi.max_entries_per_index = 10;
  isi.exclusive_reserve = 8192;
  context_t::instance().set_sparse_index_space_info(isi);
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
  auto mh = flecsi_get_mutator(ch, hydro, pressure, double, ragged, 0, 5);

  auto f1 = flecsi_execute_task_simple(task1, single, ch, mh);
  f1.wait();

  /*
    auto ch2 = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
    auto mh2 = flecsi_get_mutator(ch2, hydro, pressure, double, ragged, 0, 5);

    auto f2 = flecsi_execute_task_simple(task1b, single, ch2, mh2);
    f2.wait();
  */

  auto ph = flecsi_get_handle(ch, hydro, pressure, double, ragged, 0);

  flecsi_execute_task_simple(task2, single, ch, ph);
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(ragged_data, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

#undef DH50
