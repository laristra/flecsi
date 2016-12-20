/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// system includes
#include <cinchtest.h>
#include <iostream>
#include <string>
#include <type_traits> // std::is_same

// user includes
#include "flecsi/execution/mpilegion/legion_handshake.h"
#include "flecsi/execution/mpilegion/mapper.h"
#include "flecsi/execution/mpilegion/task_ids.h"
#include "flecsi/execution/task.h"

using namespace flecsi::execution;

extern int gtest_mpilegion_argc;
extern char** gtest_mpilegion_argv;

enum TaskIDs{
 TOP_LEVEL_TASK_ID         =0x00000100,
 HELLOWORLD_TASK_ID        =0x00000200,
};

Legion::MPILegionHandshake handshake;

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime)
{

  printf("Hello from Legion Top-Level Task\n");
  // Both the application and Legion mappers have access to
  // the mappings between MPI Ranks and Legion address spaces
  // The reverse mapping goes the other way
  const std::map<int,AddressSpace> &forward_mapping =
    runtime->find_forward_MPI_mapping();
  for (std::map<int,AddressSpace>::const_iterator it =
        forward_mapping.begin(); it != forward_mapping.end(); it++)
      printf("MPI Rank %d maps to Legion Address Space %d\n",
            it->first, it->second);
 
  int rank = -1, size = -1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  Rect<1> local_procs(Point<1>(0),Point<1>(size - 1));
  ArgumentMap arg_map;

  MustEpochLauncher must_epoch_launcher;

  IndexLauncher helloworld_launcher(HELLOWORLD_TASK_ID,
                               Domain::from_rect<1>(local_procs),
                               TaskArgument(0, 0),
                               arg_map);

  must_epoch_launcher.add_index_task(helloworld_launcher);
  FutureMap f = runtime->execute_must_epoch(ctx, must_epoch_launcher);
}




void helloworld_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
  handshake.legion_wait_on_mpi();
  printf ("helloworld \n");
  handshake.legion_handoff_to_mpi();
}

void my_init_legion(){

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

  // Create a handshake for passing control between Legion and MPI
  // Indicate that MPI has initial control and that there is one
  // participant on each side
  handshake = Runtime::create_handshake(true/*MPI initial control*/,
                                        1/*MPI participants*/,
                                        1/*Legion participants*/);

  char arguments[] = "1";
  char * argv = &arguments[0];

  const bool strict_bulk_synchronous_execution = true;
  if(strict_bulk_synchronous_execution)
    MPI_Barrier(MPI_COMM_WORLD);
  // Start the Legion runtime in background mode
  // This call will return immediately
  //HighLevelRuntime::start(1, &argv, true/*background*/);
  HighLevelRuntime::start(gtest_mpilegion_argc, gtest_mpilegion_argv, true/*background*/);
   
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
}

#define execute(task, ...) \
  execution_t::execute_task(task, ##__VA_ARGS__)

TEST(legion_handshake, simple) {

   my_init_legion(); 
 
} // TEST

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
