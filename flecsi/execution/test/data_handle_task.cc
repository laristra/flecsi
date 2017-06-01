/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#define DH2

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

using namespace flecsi;

clog_register_tag(coloring);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = 
  data::legion::dense_handle_t<T, EP, SP, GP,
  data::legion_meta_data_t<default_user_meta_data_t>>;

void task1(handle_t<double, dro, dno, dno> x, double y) {
  //np(y);
} // task1

flecsi_register_task(task1, loc, single);

class client_type : public flecsi::data::data_client_t{};

flecsi_new_register_data(client_type, ns, pressure, double, dense, 0, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_driver(int argc, char ** argv) {
  clog(info) << "In specialization driver" << std::endl;
  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_driver

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;

  client_type c;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto h = flecsi_get_handle(c, ns, pressure, double, dense, 0);

  flecsi_execute_task(task1, single, h, 128);
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(data_handle, testname) {
  
} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

#undef DH2
