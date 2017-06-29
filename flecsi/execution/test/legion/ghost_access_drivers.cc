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

#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/supplemental/coloring/add_colorings.h"

#define INDEX_ID 0
#define VERSIONS 1

clog_register_tag(ghost_access);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = flecsi::data::legion::dense_handle_t<T, EP, SP, GP>;

void check_all_cells_task(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> cell_ID,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dro> test,
        int my_color, size_t cycle);
flecsi_register_task(check_all_cells_task, flecsi::loc, flecsi::single);

void set_primary_cells_task(
        handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> cell_ID,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> test,
        int my_color, size_t cycle);
flecsi_register_task(set_primary_cells_task, flecsi::loc, flecsi::single);

class client_type : public flecsi::data::data_client_t{};

flecsi_register_field(client_type, name_space, cell_ID, size_t, dense,
    INDEX_ID, VERSIONS);
flecsi_register_field(client_type, name_space, test, double, dense,
    INDEX_ID, VERSIONS);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(trace) << "In specialization top-level-task init" << std::endl;

  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  auto runtime = Legion::Runtime::get_runtime();
  const int my_color = runtime->find_local_MPI_rank();
  clog(trace) << "Rank " << my_color << " in driver" << std::endl;

  client_type client;

  auto handle = flecsi_get_handle(client, name_space, cell_ID, size_t, dense,
      INDEX_ID);
  auto test_handle = flecsi_get_handle(client, name_space, test, double, dense,
      INDEX_ID);

  for(size_t cycle=0; cycle<3; cycle++) {
    flecsi_execute_task(set_primary_cells_task, single, handle, test_handle,
            my_color,cycle);

    flecsi_execute_task(check_all_cells_task, single, handle, test_handle,
            my_color, cycle);
  }

} // driver

} // namespace execution
} // namespace flecsi

void set_primary_cells_task(
        handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> cell_ID,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> test,
        int my_color, size_t cycle) {

  clog(trace) << "Rank " << my_color << " WRITING " << std::endl;

  flecsi::execution::context_t & context_
    = flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin();
    exclusive_itr != index_coloring->second.exclusive.end(); ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    clog(trace) << "Rank " << my_color << " exclusive " <<  exclusive.id <<
        std::endl;
    cell_ID(index) = exclusive.id + cycle;
    test(index) = double(exclusive.id + cycle);
    index++;
  } // exclusive_itr

  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    clog(trace) << "Rank " << my_color << " shared " <<  shared.id << std::endl;
    cell_ID(index) = shared.id + cycle;
    test(index) = double(shared.id + cycle);
    index++;
  } // shared_itr

  for (auto ghost_itr = index_coloring->second.ghost.begin(); ghost_itr !=
      index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
    clog(trace) << "Rank " << my_color << " ghost " <<  ghost.id << std::endl;
  } // ghost_itr

} // set_primary_cells_task

void check_all_cells_task(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> cell_ID,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dro> test,
        int my_color, size_t cycle) {
  clog(trace) << "Rank " << my_color << " READING " << std::endl;

  for (size_t i=0; i < cell_ID.exclusive_size(); i++)
      clog(trace) << "Rank " << my_color << " exclusive " << i << " = " <<
      cell_ID.exclusive(i) << std::endl;

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
    assert(test.exclusive(index) == double(exclusive.id + cycle));
    index++;
  } // exclusive_itr

  index = 0;
  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    assert(cell_ID.shared(index) == shared.id + cycle);
    assert(test.shared(index) == double(shared.id + cycle));
    index++;
  } // shared_itr

  for (size_t i=0; i < cell_ID.shared_size(); i++)
      clog(trace) << "Rank " << my_color << " shared " << i << " = " <<
      cell_ID.shared(i) << std::endl;

  for (size_t i=0; i < cell_ID.ghost_size(); i++)
      clog(trace) << "Rank " << my_color << " ghost " << i << " = " <<
      cell_ID.ghost(i) << std::endl;

  index = 0;
  for (auto ghost_itr = index_coloring->second.ghost.begin(); ghost_itr !=
      index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
    assert(cell_ID.ghost(index) == ghost.id + cycle);
    assert(test.ghost(index) == double(ghost.id + cycle));
    index++;
  } // ghost_itr

} // check_all_cells_task

TEST(ghost_access, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
