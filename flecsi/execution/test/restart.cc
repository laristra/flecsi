/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchtest.h>

#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

using namespace flecsi;
using namespace supplemental;
using mesh_t = flecsi::supplemental::test_mesh_2d_t;

//---------------------------------------------------------------------------//
// FleCSI tasks
//---------------------------------------------------------------------------//

void
write_task(data_client_handle_u<mesh_t, ro> mesh,
  dense_accessor<int, rw, rw, na> f1,
  sparse_mutator<double> f2) {
  auto & context = execution::context_t::instance();
  const auto & map = context.index_map(cells);
  for(auto c : mesh.cells(flecsi::owned)) {
    f1(c) = map.at(c.id());
    if(c.id() % 2 == 0)
      f2(c, 0) = 2 * map.at(c.id());
    else
      f2(c, 1) = 2 * map.at(c.id());
  }
} // write_task

void
clear_task(data_client_handle_u<mesh_t, ro> mesh,
  dense_accessor<int, rw, rw, na> f1,
  sparse_mutator<double> f2) {
  auto & context = execution::context_t::instance();
  const auto & map = context.index_map(cells);
  for(auto c : mesh.cells(flecsi::owned)) {
    f1(c) = 0;
    if(c.id() % 2 == 0)
      f2.erase(c, 0);
    else
      f2.erase(c, 1);
  }
} // clear_task

void
read_task(data_client_handle_u<mesh_t, ro> mesh,
  dense_accessor<int, ro, ro, ro> f1,
  sparse_accessor<double, ro, ro, ro> f2) {
  auto & context = execution::context_t::instance();
  const auto & map = context.index_map(cells);
  for(auto c : mesh.cells()) {
    EXPECT_EQ(f1(c), map.at(c.id()));
    if(c.id() % 2 == 0)
      EXPECT_EQ(f2(c, 0), 2 * map.at(c.id()));
    else
      EXPECT_EQ(f2(c, 1), 2 * map.at(c.id()));
  }
} // read_task

flecsi_register_task_simple(write_task, loc, index);
flecsi_register_task_simple(clear_task, loc, index);
flecsi_register_task_simple(read_task, loc, index);

//---------------------------------------------------------------------------//
// Data client registration
//---------------------------------------------------------------------------//
flecsi_register_data_client(mesh_t, meshes, mesh1);

//---------------------------------------------------------------------------//
// Fields
//---------------------------------------------------------------------------//
flecsi_register_field(mesh_t, fields, x, int, dense, 1, cells);
flecsi_register_field(mesh_t, fields, y, double, sparse, 1, cells);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

void
specialization_tlt_init(int argc, char ** argv) {
  supplemental::do_test_mesh_2d_coloring();

  context_t::sparse_index_space_info_t isi;
  isi.index_space = index_spaces::cells;
  isi.max_entries_per_index = 10;
  isi.exclusive_reserve = 8192;
  context_t::instance().set_sparse_index_space_info(isi);
} // specialization_tlt_init

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  auto & context = execution::context_t::instance();
  auto rank = context.color();
  std::string outfile = "restart.rst." + std::to_string(rank);

  auto ch = flecsi_get_client_handle(mesh_t, meshes, mesh1);

  auto hx = flecsi_get_handle(ch, fields, x, int, dense, 0);
  auto hym = flecsi_get_mutator(ch, fields, y, double, sparse, 0, 2);

  flecsi_execute_task_simple(write_task, index, ch, hx, hym);

  context.write_all_fields(outfile.c_str());

  flecsi_execute_task_simple(clear_task, index, ch, hx, hym);

  context.read_fields(outfile.c_str());

  auto hy = flecsi_get_handle(ch, fields, y, double, sparse, 0);
  flecsi_execute_task_simple(read_task, index, ch, hx, hy);

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(restart, testname) {} // TEST

} // namespace execution
} // namespace flecsi
