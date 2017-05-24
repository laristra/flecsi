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

clog_register_tag(coloring);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  flecsi::data::legion::dense_handle_t<T, EP, SP, GP,
  flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t>>;

void initialize_primary_cells_task(handle_t<size_t, flecsi::drw, flecsi::drw,
    flecsi::dno> cell_ID) {

  flecsi::execution::context_t & context_ = flecsi::execution::context_t::instance();
  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();

} // initialize_primary_cells_task

flecsi_register_task(initialize_primary_cells_task, flecsi::loc, flecsi::single);

class client_type : public flecsi::data::data_client_t{};

#define INDEX_ID 0
#define VERSIONS 1


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
  clog(error) << "In driver" << std::endl;

  client_type client;

  auto handle = flecsi_get_handle(client, name_space, cell_ID, size_t, dense, INDEX_ID);

  flecsi_execute_task(initialize_primary_cells_task, single, handle);
} // specialization_driver

} // namespace execution
} // namespace flecsi


TEST(ghost_access, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
