/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flecsi.h>
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion 
  #include <mpi.h>
  #include <legion.h>
#endif

#include "flecsi/execution/context.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/legion/internal_task.h"

///
// \file example_app.cc
// \authors demeshko
// \date Initial file creation: May 18, 2016
///

namespace flecsi{
namespace execution{
//----------------------------------------------------------------------------//
//Define MPI task
void
mpi_task(double a)
{ 
  std::cout<<"inside MPI task"<<std::endl;
  int rank = 0;
  int size = 0;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout << "My rank: " << rank << std::endl;
}//mpi_task

//register the task
flecsi_register_mpi_task(mpi_task);

//----------------------------------------------------------------------------//
// Define internal Legion task to register.
void internal_task_example_1(
  const Legion::Task * task,                                 
  const std::vector<Legion::PhysicalRegion> & regions,       
  Legion::Context ctx,                                       
  Legion::Runtime * runtime                         
)
{
  std::cout <<"inside of the task1" <<std::endl;
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_1,
  flecsi::processor_type_t::loc, single);

//----------------------------------------------------------------------------//
// Define internal Legion task to register.
void internal_task_example_2(
  const Legion::Task * task,                                 
  const std::vector<Legion::PhysicalRegion> & regions,       
  Legion::Context ctx,                                       
  Legion::Runtime * runtime                         
)
{
  std::cout <<"inside of the task2" <<std::endl;
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_2,
  flecsi::processor_type_t::loc, index);

//----------------------------------------------------------------------------//
//define FLeCSI task
void task1() {
  std::cout << "inside single task" <<std::endl;
} // task1

//register FLeCSI task
flecsi_register_task(task1, loc, single);

void task2(){
  std::cout<<"inside index task"<<std::endl;
}

////register FLeCSI task
flecsi_register_task(task2, loc, index);



//----------------------------------------------------------------------------//
//Define Specialization Driver : a driver that gets executed at the top level
void
specialization_tlt_init(
  int argc,
  char ** argv
)
{
  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"specialization_driver"}.hash();
  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();

  std::cout<<"inside Specialization Driver"<<std::endl;

  flecsi_execute_mpi_task( mpi_task, 0);

  flecsi_execute_task(task1, single);

  flecsi_execute_task(task2, index);

  auto key_1 = __flecsi_internal_task_key(internal_task_example_1);
  auto key_2 = __flecsi_internal_task_key(internal_task_example_2);

  //executing internal legion tasks with pure legion calls
  Legion::TaskLauncher launcher(
    context_t::instance().task_id(key_1),
    LegionRuntime::HighLevel::TaskArgument(0,0));
  auto f=runtime->execute_task(context, launcher);

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher index_launcher(
    context_t::instance().task_id(key_2),
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );

 auto fm = runtime->execute_index_space(context, index_launcher);
 fm.wait_all_results();

}//specialization_driver

//----------------------------------------------------------------------------//
void
driver(
  int argc,
  char ** argv
)
{
  std::cout<<"inside Driver"<<std::endl;

  flecsi_execute_task(task1, single);
  
  flecsi_execute_task(task2, index);
}//driver

}//namespace execution
}//namespace flecsi

//----------------------------------------------------------------------------//

int main(int argc, char ** argv) {


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

  std::cout <<"taks and drivers exmple"<<std::endl;

  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif 

  return retval;
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
