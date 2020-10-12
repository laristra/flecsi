/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchtest.h>

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>
#include <flecsi/utils/fixed_vector.h>

using namespace flecsi;
using namespace supplemental;
using mesh_t = flecsi::supplemental::test_mesh_2d_t;

//---------------------------------------------------------------------------//
// FleCSI tasks
//---------------------------------------------------------------------------//

void
write_task(data_client_handle_u<mesh_t, ro> mesh,
  std::tuple<dense_accessor<int, rw, rw, na>, dense_accessor<int, rw, rw, na>>
    fs) {
  auto & context = execution::context_t::instance();
  const auto & map = context.index_map(cells);
  for(auto c : mesh.cells(flecsi::owned)) {
    std::get<0>(fs)(c) = static_cast<int>(map.at(c.id()));
    std::get<1>(fs)(c) = static_cast<int>(2 * map.at(c.id()));
  }
} // task1

void
read_task(data_client_handle_u<mesh_t, ro> mesh,
  std::tuple<dense_accessor<int, ro, ro, ro>, dense_accessor<int, ro, ro, ro>>
    fs) {
  auto & context = execution::context_t::instance();
  const auto & map = context.index_map(cells);
  for(auto c : mesh.cells()) {
    EXPECT_EQ(std::get<0>(fs)(c), map.at(c.id()));
    EXPECT_EQ(std::get<1>(fs)(c), 2 * map.at(c.id()));
  }
} // task1

flecsi_register_task_simple(write_task, loc, index);
flecsi_register_task_simple(read_task, loc, index);

//---------------------------------------------------------------------------//
// Data client registration
//---------------------------------------------------------------------------//
flecsi_register_data_client(mesh_t, meshes, mesh1);

//---------------------------------------------------------------------------//
// Fields
//---------------------------------------------------------------------------//
flecsi_register_field(mesh_t, fields, x, int, dense, 1, cells);
flecsi_register_field(mesh_t, fields, y, int, dense, 1, cells);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

void
specialization_tlt_init(int argc, char ** argv) {
  supplemental::do_test_mesh_2d_coloring();
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
  auto ch = flecsi_get_client_handle(mesh_t, meshes, mesh1);

  auto hx = flecsi_get_handle(ch, fields, x, int, dense, 0);
  auto hy = flecsi_get_handle(ch, fields, y, int, dense, 0);
  auto hs = std::make_tuple(hx, hy);

  flecsi_execute_task_simple(write_task, index, ch, hs);
  flecsi_execute_task_simple(read_task, index, ch, hs);

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(handle_list, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
