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
using client_handle_t = data_client_handle_u<DC, PS>;

void
init(client_handle_t<test_mesh_t, ro> mesh, sparse_mutator<double> sm) {
  auto rank = execution::context_t::instance().color();

  for(auto c : mesh.cells(owned)) {
    auto gid = c->gid();
    // for most cells, do a checkerboard pattern
    bool parity = (gid / 8 + gid % 8) & 1;
    int start = (parity ? 0 : 1);
    int stop = (parity ? 6 : 5);
    // make a few cells overflow
    if(gid >= 11 && gid <= 13)
      stop = (parity ? 16 : 17);
    for(size_t j = start; j < stop; j += 2) {
      sm(c, j) = static_cast<double>(rank * 10000 + gid * 100 + j);
    }
  }
} // init

void
modify(client_handle_t<test_mesh_t, ro> mesh,
  sparse_accessor<double, rw, rw, ro> sh) {
  for(auto c : mesh.cells(owned)) {
    for(auto entry : sh.entries(c)) {
      sh(c, entry) = -sh(c, entry);
    }
  }
} // modify

void
mutate(client_handle_t<test_mesh_t, ro> mesh, sparse_mutator<double> sm) {
  auto rank = execution::context_t::instance().color();

  for(auto c : mesh.cells(owned)) {
    auto gid = c->gid();
    bool parity = (gid / 8 + gid % 8) & 1;
    // make some cells overflow
    if(gid == 11 || gid == 14) {
      int start = (parity ? 6 : 5);
      int stop = (parity ? 20 : 21);
      for(size_t j = start; j < stop; j += 2) {
        sm(c, j) = static_cast<double>(rank * 10000 + gid * 100 + 50 + j);
      }
      sm.erase(c, start - 4);
      sm.erase(c, stop - 4);
    }
    else if(gid == 13) {
      sm.erase(c, 9);
    }
    else if(parity) {
      sm(c, 6) = static_cast<double>(rank * 10000 + gid * 100 + 66);
    }
    else {
      sm(c, 3) = static_cast<double>(rank * 10000 + gid * 100 + 77);
    }
  }
} // mutate

void
print(client_handle_t<test_mesh_t, ro> mesh,
  sparse_accessor<double, ro, ro, ro> sh) {
  for(auto c : mesh.cells()) {
    for(auto entry : sh.entries(c)) {
      CINCH_CAPTURE() << c->id() << ":" << entry << ": " << sh(c, entry)
                      << std::endl;
    }
  }
} // print

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_task_simple(init, loc, index);
flecsi_register_task_simple(modify, loc, index);
flecsi_register_task_simple(mutate, loc, index);
flecsi_register_task_simple(print, loc, index);

flecsi_register_field(test_mesh_t,
  hydro,
  pressure,
  double,
  sparse,
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
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto pm = flecsi_get_mutator(ch, hydro, pressure, double, sparse, 0, 5);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, sparse, 0);

  flecsi_execute_task_simple(init, index, ch, pm);
  flecsi_execute_task_simple(print, index, ch, ph);

  flecsi_execute_task_simple(modify, index, ch, ph);
  flecsi_execute_task_simple(print, index, ch, ph);

  flecsi_execute_task_simple(mutate, index, ch, pm);
  auto future = flecsi_execute_task_simple(print, index, ch, ph);
  future.wait(); // wait before comparing results

  auto & context = execution::context_t::instance();
  if(context.color() == 0) {
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("sparse_data.blessed"));
  }
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(sparse_data, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
