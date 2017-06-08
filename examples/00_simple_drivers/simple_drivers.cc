/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#include <iostream>
#include <flecsi.h>
#if ENABLE_MPI
#include <mpi.h>
#endif

#include "flecsi/execution/context.h"

///
// \file example_driver.h
// \authors bergen
// \date Initial file creation: Aug 25, 2016
///

namespace flecsi {
namespace execution {

void
specialization_driver(
  int argc,
  char ** argv
) 
{ 
  std::cout<<"inside Specialization Driver"<<std::endl;
}//specialization_driver

void
driver(
  int argc,
  char ** argv
)
{
  std::cout << "Hello World" << std::endl;
} // driver

} // namespace
} // namespace

int main(int argc, char ** argv) {

  int provided;
#if ENABLE_MPI
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  // If you fail this assertion, then your version of MPI
  // does not support calls from multiple threads and you
  // cannot use the GASNet MPI conduit
  if (provided < MPI_THREAD_MULTIPLE)
    printf("ERROR: Your implementation of MPI does not support "
           "MPI_THREAD_MULTIPLE which is required for use of the "
           "GASNet MPI conduit with the Legion-MPI Interop!\n");
  assert(provided == MPI_THREAD_MULTIPLE);
#endif
  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

  return retval;
} // main


/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
