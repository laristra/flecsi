/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 4, 2017
//----------------------------------------------------------------------------//

#include <chrono>
#include <thread>

#include <cinchlog.h>
#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/data/data.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/empty_mesh_2d.h>

#define INDEX_ID 0
#define VERSIONS 1

using namespace flecsi;
using namespace supplemental;

clog_register_tag(barrier_per_field);

void read_task(
        dense_accessor<size_t, flecsi::ro, flecsi::ro, flecsi::ro> cell_ID,
        const int my_color, const size_t cycle);
flecsi_register_task_simple(read_task, loc, single);

void write_task(
        dense_accessor<size_t, flecsi::rw, flecsi::rw, flecsi::ro> cell_ID,
        const int my_color, const size_t cycle, const bool delay);
flecsi_register_task_simple(write_task, loc, single);

flecsi_register_data_client(empty_mesh_2d_t, meshes, mesh1);

flecsi_register_field(empty_mesh_2d_t, name_space, field1, size_t, dense,
    VERSIONS, INDEX_ID);
flecsi_register_field(empty_mesh_2d_t, name_space, field2, size_t, dense,
    VERSIONS, INDEX_ID);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(trace) << "In specialization top-level-task init" << std::endl;

  coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;

  flecsi_execute_mpi_task(add_colorings, flecsi::execution, map);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  auto runtime = Legion::Runtime::get_runtime();
  const int my_color = runtime->find_local_MPI_rank();
  clog(trace) << "Rank " << my_color << " in driver" << std::endl;

  auto ch = flecsi_get_client_handle(empty_mesh_2d_t, meshes, mesh1);

  auto handle1 = flecsi_get_handle(ch, name_space,field1, size_t, dense,
      INDEX_ID);
  auto handle2 = flecsi_get_handle(ch, name_space,field2, size_t, dense,
      INDEX_ID);

  for(size_t cycle=0; cycle<3; cycle++) {
    bool delay = false;
    flecsi_execute_task_simple(write_task, single, handle1, my_color,
      cycle, delay);

    delay = true;
    flecsi_execute_task_simple(write_task, single, handle2, my_color,
      cycle, delay);

    flecsi_execute_task_simple(read_task, single, handle2, my_color, cycle);

    flecsi_execute_task_simple(read_task, single, handle1, my_color, cycle);

    flecsi_execute_task_simple(read_task, single, handle2, my_color, cycle);
  }

  // Permutation test
  bool delay = false;
  flecsi_execute_task_simple(write_task, single, handle1, my_color, 0, delay);
  flecsi_execute_task_simple(write_task, single, handle2, my_color, 0, delay);
  flecsi_execute_task_simple(read_task, single, handle1, my_color, 0);
  flecsi_execute_task_simple(read_task, single, handle2, my_color, 0);

  flecsi_execute_task_simple(write_task, single, handle2, my_color, 1, delay);
  flecsi_execute_task_simple(read_task, single, handle1, my_color, 0);
  flecsi_execute_task_simple(read_task, single, handle2, my_color, 1);

  flecsi_execute_task_simple(write_task, single, handle1, my_color, 2, delay);
  flecsi_execute_task_simple(read_task, single, handle1, my_color, 2);
  flecsi_execute_task_simple(read_task, single, handle2, my_color, 1);

} // driver

} // namespace execution
} // namespace flecsi

void write_task(
        dense_accessor<size_t, flecsi::rw, flecsi::rw, flecsi::ro> cell_ID,
        const int my_color,
        const size_t cycle,
        const bool delay) {

  if(delay)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  flecsi::execution::context_t & context_
    = flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin();
    exclusive_itr != index_coloring->second.exclusive.end(); ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    cell_ID.exclusive(index) = exclusive.id + cycle;
    index++;
  } // exclusive_itr

  index =0;
  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    cell_ID.shared(index) = shared.id + cycle;
    index++;
  } // shared_itr
} // write_task

void read_task(
        dense_accessor<size_t, flecsi::ro, flecsi::ro, flecsi::ro> cell_ID,
        const int my_color, const size_t cycle) {

  flecsi::execution::context_t & context_
    = flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin();
      exclusive_itr != index_coloring->second.exclusive.end(); ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    assert(cell_ID.exclusive(index) == exclusive.id + cycle);
    index++;
  } // exclusive_itr

  index = 0;
  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    assert(cell_ID.shared(index) == shared.id + cycle);
    index++;
  } // shared_itr

  index = 0;
  for (auto ghost_itr = index_coloring->second.ghost.begin(); ghost_itr !=
      index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
    assert(cell_ID.ghost(index) == ghost.id + cycle);
    index++;
  } // ghost_itr

} // read_task

TEST(barrier_per_field, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
