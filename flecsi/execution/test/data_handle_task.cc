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
  data::legion::dense_handle_t<T, EP, SP, GP>;

void task1(handle_t<double, dro, dno, dno> x, double y) {
  //np(y);
} // task1

void writer(handle_t<double, dwd, dno, dno> x) {
  clog(info) << "writer write: " << std::endl;
  for (int i = 0; i < 5; i++) {
    x(i) = static_cast<double>(i);
    clog(info) << x(i) << std::endl;
  }
}

void reader(handle_t<double, dro, dno, dno> x) {
  clog(info) << "reader read: " << std::endl;
  for (int i = 0; i < 5; i++) {
    clog(info) << x(i) << std::endl;
  }
}

flecsi_register_task(task1, loc, single);
flecsi_register_task(writer, loc, single);
flecsi_register_task(reader, loc, single);

class client_type : public flecsi::data::data_client_t{};

flecsi_register_field(client_type, ns, pressure, double, dense, 0, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_tlt_init

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
  flecsi_execute_task(writer, single, h);
  flecsi_execute_task(reader, single, h);
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
