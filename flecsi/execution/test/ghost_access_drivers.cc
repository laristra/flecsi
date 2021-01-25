/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: May 4, 2017
///

#include <cinchlog.h>
#include <cinchtest.h>

#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/empty_mesh_2d.h>

#define INDEX_ID 0
#define VERSIONS 1

using namespace flecsi;
using namespace supplemental;
using namespace topology;

clog_register_tag(ghost_access);

flecsi_register_data_client(empty_mesh_t, meshes, mesh1);

void check_all_cells_task(
  dense_accessor<size_t, flecsi::ro, flecsi::ro, flecsi::ro> cell_ID,
  dense_accessor<double, flecsi::ro, flecsi::ro, flecsi::ro> test,
  size_t cycle);
flecsi_register_task_simple(check_all_cells_task, loc, index | leaf);

void set_primary_cells_task(
  dense_accessor<size_t, flecsi::rw, flecsi::rw, flecsi::na> cell_ID,
  dense_accessor<double, flecsi::rw, flecsi::rw, flecsi::na> test,
  size_t cycle);
flecsi_register_task_simple(set_primary_cells_task, loc, index | leaf);

flecsi_register_field(empty_mesh_t,
  name_space,
  cell_ID,
  size_t,
  dense,
  VERSIONS,
  INDEX_ID);
flecsi_register_field(empty_mesh_t,
  name_space,
  test,
  double,
  dense,
  VERSIONS,
  INDEX_ID);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(trace) << "In specialization top-level-task init" << std::endl;

  coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;

  flecsi_execute_mpi_task(add_colorings, flecsi::supplemental, map);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  clog(trace) << " in driver" << std::endl;

  auto ch = flecsi_get_client_handle(empty_mesh_t, meshes, mesh1);

  auto handle =
    flecsi_get_handle(ch, name_space, cell_ID, size_t, dense, INDEX_ID);
  auto test_handle =
    flecsi_get_handle(ch, name_space, test, double, dense, INDEX_ID);

  for(size_t cycle = 0; cycle < 3; cycle++) {
    flecsi_execute_task_simple(
      set_primary_cells_task, index, handle, test_handle, cycle);

    auto future = flecsi_execute_task_simple(
      check_all_cells_task, index, handle, test_handle, cycle);
    future.get(); // make sure that next iteration doesn't start prematurely
  }

} // driver

} // namespace execution
} // namespace flecsi

void
set_primary_cells_task(
  dense_accessor<size_t, flecsi::rw, flecsi::rw, flecsi::na> cell_ID,
  dense_accessor<double, flecsi::rw, flecsi::rw, flecsi::na> test,
  size_t cycle) {

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map =
    context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for(auto exclusive_itr = index_coloring->second.exclusive.begin();
      exclusive_itr != index_coloring->second.exclusive.end();
      ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    cell_ID.exclusive(index) = exclusive.id + cycle;
    test.exclusive(index) = double(exclusive.id + cycle);
    index++;
  } // exclusive_itr

  index = 0;
  for(auto shared_itr = index_coloring->second.shared.begin();
      shared_itr != index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    cell_ID.shared(index) = shared.id + cycle;
    test.shared(index) = double(shared.id + cycle);
    index++;
  } // shared_itr

  for(auto ghost_itr = index_coloring->second.ghost.begin();
      ghost_itr != index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
  } // ghost_itr

} // set_primary_cells_task

void
check_all_cells_task(
  dense_accessor<size_t, flecsi::ro, flecsi::ro, flecsi::ro> cell_ID,
  dense_accessor<double, flecsi::ro, flecsi::ro, flecsi::ro> test,
  size_t cycle) {

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map =
    context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for(auto exclusive_itr = index_coloring->second.exclusive.begin();
      exclusive_itr != index_coloring->second.exclusive.end();
      ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    ASSERT_EQ(cell_ID.exclusive(index), exclusive.id + cycle);
    ASSERT_EQ(test.exclusive(index), double(exclusive.id + cycle));
    index++;
  } // exclusive_itr

  index = 0;
  for(auto shared_itr = index_coloring->second.shared.begin();
      shared_itr != index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    ASSERT_EQ(cell_ID.shared(index), shared.id + cycle);
    ASSERT_EQ(test.shared(index), double(shared.id + cycle));
    index++;
  } // shared_itr

  index = 0;
  for(auto ghost_itr = index_coloring->second.ghost.begin();
      ghost_itr != index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
    ASSERT_EQ(cell_ID.ghost(index), ghost.id + cycle);
    ASSERT_EQ(test.ghost(index), double(ghost.id + cycle));
    index++;
  } // ghost_itr

} // check_all_cells_task

TEST(ghost_access, testname) {} // TEST

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
