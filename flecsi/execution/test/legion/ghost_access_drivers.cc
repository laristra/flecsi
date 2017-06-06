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
#include "flecsi/execution/context.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/coloring_types.h"
#include "flecsi/coloring/communicator.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/coloring/parmetis_colorer.h"
#include "flecsi/coloring/mpi_communicator.h"
#include "flecsi/topology/closure_utils.h"
#include "flecsi/utils/set_utils.h"
#include "flecsi/data/data.h"
#include "flecsi/supplemental/coloring/add_colorings.h"

#define INDEX_ID 0
#define VERSIONS 1

clog_register_tag(ghost_access);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  flecsi::data::legion::dense_handle_t<T, EP, SP, GP,
  flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t>>;


void check_all_cells_task(handle_t<size_t, flecsi::dro, flecsi::dro,
    flecsi::dro> cell_ID, int my_color, size_t cycle) {
  clog(trace) << my_color << " READING " << std::endl;

  for (size_t i=0; i < cell_ID.exclusive_size(); i++)
      clog(trace) << my_color << " exclusive " << i << " = " << cell_ID.exclusive(i) <<
      std::endl;

  flecsi::execution::context_t & context_ = flecsi::execution::context_t::instance();
  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin(); exclusive_itr !=
      index_coloring->second.exclusive.end(); ++exclusive_itr) {
      flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    assert(cell_ID.exclusive(index) == exclusive.id);
    index++;
  } // exclusive_itr

  index = 0;
  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
      flecsi::coloring::entity_info_t shared = *shared_itr;
      assert(cell_ID.shared(index) == shared.id);
      index++;
  } // shared_itr

  for (size_t i=0; i < cell_ID.shared_size(); i++)
      clog(trace) << my_color << " shared " << i << " = " << cell_ID.shared(i) <<
      std::endl;

  for (size_t i=0; i < cell_ID.ghost_size(); i++)
      clog(trace) << my_color << " ghost " << i << " = " << cell_ID.ghost(i) <<
      std::endl;

  index = 0;
  for (auto ghost_itr = index_coloring->second.ghost.begin(); ghost_itr !=
      index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
    assert(cell_ID.ghost(index) == ghost.id);
    index++;
  } // ghost_itr

} // initialize_primary_cells_task

flecsi_register_task(check_all_cells_task, flecsi::loc, flecsi::single);

void initialize_primary_cells_task(handle_t<size_t, flecsi::drw, flecsi::drw,
    flecsi::dro> cell_ID, int my_color) {

  clog(trace) << my_color << " WRITING " << std::endl;

  flecsi::execution::context_t & context_ = flecsi::execution::context_t::instance();
  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin(); exclusive_itr !=
      index_coloring->second.exclusive.end(); ++exclusive_itr) {
      flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    clog(trace) << my_color << " exclusive " <<  exclusive.id << std::endl;
    cell_ID(index) = exclusive.id;
    index++;
  } // exclusive_itr

  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
      flecsi::coloring::entity_info_t shared = *shared_itr;
    clog(trace) << my_color << " shared " <<  shared.id << std::endl;
    cell_ID(index) = shared.id;
    index++;
  } // shared_itr

  for (auto ghost_itr = index_coloring->second.ghost.begin(); ghost_itr !=
      index_coloring->second.ghost.end(); ++ghost_itr) {
    flecsi::coloring::entity_info_t ghost = *ghost_itr;
    clog(trace) << my_color << " ghost " <<  ghost.id << std::endl;
    //cell_ID(index) = ghost.id;
    index++;
  } // ghost_itr

} // initialize_primary_cells_task

flecsi_register_task(initialize_primary_cells_task, flecsi::loc, flecsi::single);

class client_type : public flecsi::data::data_client_t{};

flecsi_new_register_data(client_type, name_space, cell_ID, size_t, dense, INDEX_ID, VERSIONS);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_driver(int argc, char ** argv) {
  clog(trace) << "In specialization driver" << std::endl;

  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_driver

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(trace) << "In driver" << std::endl;

#if defined(ENABLE_LEGION_TLS)
  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();
#else
  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);
#endif

  const int my_color = runtime->find_local_MPI_rank();

  client_type client;

  auto handle = flecsi_get_handle(client, name_space, cell_ID, size_t, dense, INDEX_ID);

  for(size_t cycle=0; cycle<1; cycle++) {
    flecsi_execute_task(initialize_primary_cells_task, single, handle, my_color);

    flecsi_execute_task(check_all_cells_task, single, handle, my_color, cycle);
  }

} // specialization_driver

} // namespace execution
} // namespace flecsi


TEST(ghost_access, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
