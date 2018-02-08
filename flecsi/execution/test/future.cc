/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Oct 17, 2017
///


#include <cinchtest.h>

#include "flecsi/execution/execution.h"
#include "flecsi/execution/legion/future.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/supplemental/mesh/empty_mesh_2d.h"

using namespace flecsi;
using namespace supplemental;

clog_register_tag(coloring);


template<typename T>
using handle_t = flecsi::execution::legion_future__<T, Legion::Future>;


void future_handle_dump(handle_t<double> x) {
 double tmp = x.data();
  std::cout << "future_handle =  " << tmp << std::endl;
}


double writer( double something) {
  clog(info) << "writer write" << std::endl;
  double x = 3.14;
  return x;
}

void reader(handle_t<double> x, handle_t<double> y) {
  clog(info) << "reader read: " << std::endl;
  ASSERT_EQ(x.data(), static_cast<double>(3.14));
  ASSERT_EQ(x.data(),y.data());
}

flecsi_register_task(writer, , loc, single);
flecsi_register_task(reader, , loc, single);
flecsi_register_task(future_handle_dump, , loc, single);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

  auto & context = execution::context_t::instance();
 
  ASSERT_EQ(context.execution_state(),
    static_cast<size_t>(SPECIALIZATION_TLT_INIT));

#if 0
  flecsi_execute_task(reader, single, future);
#endif
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;

  auto & context = execution::context_t::instance();
  ASSERT_EQ(context.execution_state(), static_cast<size_t>(DRIVER));

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto future  = flecsi_execute_task(writer, , single, 0.0);
  int i;
  i=0;;
   future.wait();
  flecsi_execute_task(future_handle_dump, , single, future);

  flecsi_execute_task(reader, , single, future, future);
} // driver

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
