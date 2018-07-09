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
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

clog_register_tag(coloring);

namespace flecsi {
namespace execution {

using test_mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void
init(client_handle_t<test_mesh_t, ro> mesh, ragged_mutator<double> rm) {
  auto rank = execution::context_t::instance().color();

  for (auto c : mesh.cells(owned)) {
    rm.resize(c, 3);
    for (size_t j = 0; j < 3; ++j) {
      rm(c, j) = c->id() * 100 + j + rank * 10000;
    }
  }

  for (auto c : mesh.cells(owned)) {
    rm.push_back(c, c->id() * 100 + 4 + rank * 10000);
    rm.insert(c, 3, c->id() * 100 + 3 + rank * 10000);
  }
} // init

void
print(
    client_handle_t<test_mesh_t, ro> mesh,
    ragged_accessor<double, ro, ro, ro> rh) {
  for (auto c : mesh.cells()) {
    auto index = c->id();
    // FIXME add size member function for ragged_accessor
    for (size_t e = 0; e < rh.entries(index).size(); ++e) {
      CINCH_CAPTURE() << index << ":" << e << ": " << rh(index, e) << std::endl;
    }
  }
} // print

void
modify(
    client_handle_t<test_mesh_t, ro> mesh,
    ragged_accessor<double, rw, rw, rw> rh) {
  for (auto c : mesh.cells(owned)) {
    // FIXME add size member function for ragged_accessor
    for (size_t e = 0; e < rh.entries(c).size(); ++e) {
      rh(c, e) = -rh(c, e);
    }
  }
} // modify

void
mutate(client_handle_t<test_mesh_t, ro> mesh, ragged_mutator<double> rm) {
  auto rank = execution::context_t::instance().color();
  for (auto c : mesh.cells(owned)) {
    rm.erase(c, 1);
    rm.push_back(c, c->id() * 100 + 5 + rank * 10000);
    rm.insert(c, 1, c->id() * 100 + 1 + rank * 10000);
  }
} // mutate

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_task_simple(init, loc, single);
flecsi_register_task_simple(print, loc, single);
flecsi_register_task_simple(modify, loc, single);
flecsi_register_task_simple(mutate, loc, single);

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
  auto mh = flecsi_get_mutator(ch, hydro, pressure, double, ragged, 0, 10);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, ragged, 0);

  flecsi_execute_task_simple(init, single, ch, mh);
  flecsi_execute_task_simple(print, single, ch, ph);

  flecsi_execute_task_simple(modify, single, ch, ph);
  flecsi_execute_task_simple(print, single, ch, ph);

  flecsi_execute_task_simple(mutate, single, ch, mh);
  auto future = flecsi_execute_task_simple(print, single, ch, ph);
  future.wait(); // wait before comparing results

  auto & context = execution::context_t::instance();
  if (context.color() == 0) {
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("ragged_data.blessed"));
  }
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
