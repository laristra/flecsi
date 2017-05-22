/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flecsi.h>
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion 
  #include <mpi.h>
  #include <legion.h>
#endif

#include "flecsi/execution/execution.h"
#include "flecsi/execution/common/processor.h"


///
// \file example_app.cc
// \authors demeshko
// \date Initial file creation: May 18, 2016
///

using namespace flecsi;
using namespace flecsi::execution;

void
mpi_task(double &a)
{ 
  std::cout<<"inside MPI task"<<std::endl;
  int rank = 0;
  int size = 0;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout << "My rank: " << rank << std::endl;
}//mpi_task


flecsi_register_mpi_task(mpi_task);



namespace flecsi{
namespace execution{
void
specialization_driver(
  int argc,
  char ** argv
)
{
  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"specialization_driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);

  std::cout<<"inside Specialization Driver"<<std::endl;

 // flecsi_execute_mpi_task(mpi_task);
}//specialization_driver

#if 0
void task1( float y) {
  std::cout << "inside single task" <<std::endl;
} // task1

flecsi_register_task(task1, flecsi::execution::processor_type_t::loc, flecsi::single);

void task2(){
  std::cout<<"inside index task"<<std::endl;
}

flecsi_register_task(task2, flecsi::execution::processor_type_t::loc, flecsi::index);
#endif

void
driver(
  int argc,
  char ** argv
)
{
  std::cout<<"inside Driver"<<std::endl;
#if 0
  flecsi_execute_task(task1, flecsi::single);
  
  flecsi_execute_task(task2, flecsi::index);
#endif
}//driver

}
}

int main(int argc, char ** argv) {

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#ifdef GASNET_CONDUIT_MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    // If you fail this assertion, then your version of MPI
    // does not support calls from multiple threads and you
    // cannot use the GASNet MPI conduit
    if (provided < MPI_THREAD_MULTIPLE)
      printf("ERROR: Your implementation of MPI does not support "
           "MPI_THREAD_MULTIPLE which is required for use of the "
           "GASNet MPI conduit with the Legion-MPI Interop!\n");
    assert(provided == MPI_THREAD_MULTIPLE);
#else
std::cout <<"MPI MPI  MPI"<< std::endl;
   MPI_Init(&argc, &argv);
#endif
#endif

  std::cout <<"taks and drivers exmple"<<std::endl;

  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion 
#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif 
#endif

  return retval;
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
