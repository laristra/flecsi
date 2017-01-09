/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_test_handshake_init_h
#define flecsi_execution_test_handshake_init_h

// system includes
#include <cinchtest.h>
#include <iostream>
#include <string>
#include <type_traits> // std::is_same

// user includes
#include "flecsi/execution/mpilegion/legion_handshake.h"
#include "flecsi/execution/mpilegion/mapper.h"
#include "flecsi/execution/task_ids.h"
#include "flecsi/execution/task.h"

using namespace flecsi::execution;

enum TaskIDs{
 TOP_LEVEL_TASK_ID         =0x00000100,
 HELLOWORLD_TASK_ID        =0x00000200,
};


using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime);

void helloworld_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime);



///
/// \file
/// \date Initial file creation: Dec 21, 2016
///

void
inline
test_init(
  int argc,
  char ** argv
)
{
  int rank = -1, size = -1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  printf("Hello from MPI process %d of %d\n", rank, size);

  // Configure the Legion runtime with the rank of this process
  Legion::Runtime::configure_MPI_interoperability(rank);
  // Register our task variants
  {
    TaskVariantRegistrar top_level_registrar(TOP_LEVEL_TASK_ID);
    top_level_registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<top_level_task>(top_level_registrar,
                                                      "Top Level Task");
    Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  }
  {
    TaskVariantRegistrar helloworld_registrar(HELLOWORLD_TASK_ID);
    helloworld_registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<helloworld_mpi_task>(helloworld_registrar,
                                                        "MPI Interop Task");
  }

   flecsi::execution::ext_legion_handshake_t & handshake=
     flecsi::execution::ext_legion_handshake_t::instance();
  // Create a handshake for passing control between Legion and MPI
  // Indicate that MPI has initial control and that there is one
  // participant on each side

    handshake.initialize();

  // This will initialize the legion runtime, so you need to
  // do anything else before you get here.
     HighLevelRuntime::start(argc, argv, true/*background*/);

  const bool strict_bulk_synchronous_execution = true;
  if(strict_bulk_synchronous_execution)
    MPI_Barrier(MPI_COMM_WORLD);
  // Start the Legion runtime in background mode
  // This call will return immediately
  //HighLevelRuntime::start(argc, argv, true/*background*/);

  // Perform a handoff to Legion, this call is
  // asynchronous and will return immediately
  handshake.mpi_handoff_to_legion();
  // You can put additional work in here if you like
  // but it may interfere with Legion work

  // Wait for Legion to hand control back,
  // This call will block until a Legion task
  // running in this same process hands control back
  handshake.mpi_wait_on_legion();
  if(strict_bulk_synchronous_execution)
    MPI_Barrier(MPI_COMM_WORLD);

  // When you're done wait for the Legion runtime to shutdown
  Runtime::wait_for_shutdown();
  // Then finalize MPI like normal 

} // test_init

#endif // flecsi_execution_task_init_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
