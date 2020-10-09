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
init(client_handle_t<test_mesh_t, ro> mesh, ragged_mutator<double> rm) {
  auto rank = execution::context_t::instance().color();

  for(auto c : mesh.cells(owned)) {
    auto gid = c->gid();
    // for most cells, do a checkerboard pattern
    bool parity = (gid / 8 + gid % 8) & 1;
    int count = (parity ? 3 : 2);
    // make a few cells overflow
    if(gid >= 11 && gid <= 13)
      count = 8;
    rm.resize(c, count);
    for(size_t j = 0; j < count; ++j) {
      rm(c, j) = static_cast<double>(rank * 10000 + gid * 100 + j);
    }
  }
} // init

void
modify(client_handle_t<test_mesh_t, ro> mesh,
  ragged_accessor<double, rw, rw, ro> rh) {
  for(auto c : mesh.cells(owned)) {
    size_t rsize = rh.size(c);
    for(size_t r = 0; r < rsize; ++r) {
      rh(c, r) = -rh(c, r);
    }
  }
} // modify

void
mutate(client_handle_t<test_mesh_t, ro> mesh, ragged_mutator<double> rm) {
  auto rank = execution::context_t::instance().color();

  for(auto c : mesh.cells(owned)) {
    auto gid = c->gid();
    bool parity = (gid / 8 + gid % 8) & 1;
    // make some cells overflow
    if(gid == 11 || gid == 14) {
      rm.resize(c, 10);
      for(size_t j = 3; j < 10; ++j) {
        rm(c, j) = static_cast<double>(rank * 10000 + gid * 100 + 50 + j);
      }
      rm.erase(c, 1);
    }
    else if(gid == 13) {
      auto n = rank * 10000 + gid * 100 + 66;
      rm.push_back(c, static_cast<double>(n));
      rm.insert(c, 1, static_cast<double>(n + 1));
    }
    // flip the checkerboard:  entries that had 3 entries will now
    // have 2, and vice-versa
    else if(parity) {
      rm.resize(c, 2);
      rm(c, 1) = -rm(c, 1) + 70;
    }
    else {
      auto n = rank * 10000 + gid * 100 + 88;
      rm.insert(c, 1, static_cast<double>(n));
      rm(c, 2) = -rm(c, 2) + 80;
    }
  }
} // mutate

void
print(client_handle_t<test_mesh_t, ro> mesh,
  ragged_accessor<double, ro, ro, ro> rh) {
  for(auto c : mesh.cells()) {
    size_t rsize = rh.size(c);
    for(size_t r = 0; r < rsize; ++r) {
      CINCH_CAPTURE() << c->id() << ":" << r << ": " << rh(c, r) << std::endl;
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
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto pm = flecsi_get_mutator(ch, hydro, pressure, double, ragged, 0, 5);
  auto ph = flecsi_get_handle(ch, hydro, pressure, double, ragged, 0);

  flecsi_execute_task_simple(init, index, ch, pm);
  flecsi_execute_task_simple(print, index, ch, ph);

  flecsi_execute_task_simple(modify, index, ch, ph);
  flecsi_execute_task_simple(print, index, ch, ph);

  flecsi_execute_task_simple(mutate, index, ch, pm);
  auto future = flecsi_execute_task_simple(print, index, ch, ph);
  future.wait(); // wait before comparing results

  auto & context = execution::context_t::instance();
  if(context.color() == 0) {
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
