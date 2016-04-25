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
#include "flecsi/utils/mpi_legion_interoperability/legion_handshake.h"
#include "flecsi/execution/mpi_execution_policy.h"
#include "flecsi/execution/task.h"

using execution_t = flecsi::execution_t<flecsi::mpi_execution_policy_t>;
using return_type_t = execution_t::return_type_t;

enum TaskIDs{
 TOP_LEVEL_TASK_ID,
 CONNECT_MPI_TASK_ID,
 HELLOWORLD_TASK_ID,
};

ExtLegionHandshake *handshake;

return_type_t world_size() {
#ifdef DEBUG
  printf ("inside MPI function \n");
#endif 
  int world_size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  return 0;
}

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime)
{
  int num_procs=1;
#ifdef DEBUG
  printf ("inside top_level_task function \n");
#endif

//TOFIX: change to the correct allocation
//what "all_procs" index space represents?
  Rect<2> all_procs(Point<2>::ZEROES(),make_point(num_procs, num_procs));
  ArgumentMap arg_map;

  IndexLauncher connect_mpi_launcher(CONNECT_MPI_TASK_ID,
                                       Domain::from_rect<2>(all_procs),
                                       TaskArgument(0, 0),
                                       arg_map);
  IndexLauncher helloworld_launcher(HELLOWORLD_TASK_ID,
                               Domain::from_rect<2>(all_procs),
                               TaskArgument(0, 0),
                               arg_map);
  //run legion_init() from each thead
  runtime->execute_index_space(ctx, connect_mpi_launcher);
  runtime->execute_index_space(ctx, helloworld_launcher);

  printf("connect_mpi finished \n");

    //run some legion task here

}

void connect_mpi_task (const Task *task,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
#ifdef DEBUG
  printf ("inside connect_mpi_task \n");
#endif
    handshake->legion_init();
}


void helloworld_mpi_task (const Task *task,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
  printf ("helloworld \n");
}

void complete_legion_configure(void)
{
#ifdef DEBUG
  printf ("inside complete_legion_configure function \n");
#endif
   handshake->ext_init();
}

void run_legion_task(void)
{
#ifdef DEBUG
  printf ("inside run_legion_task function \n");
#endif
   handshake->ext_handoff_to_legion();
 //  handshake->ext_wait_on_legion();  // Legion running!!! Yeah !! 
}

void my_init_legion(){

  handshake = new ExtLegionHandshake(ExtLegionHandshake::IN_EXT, 1, 1);

  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);

  HighLevelRuntime::register_legion_task<top_level_task>( TOP_LEVEL_TASK_ID,
                          Processor::LOC_PROC, true/*single*/, false/*index*/, 
                          AUTO_GENERATE_ID, TaskConfigOptions(), "top_level_task");
  HighLevelRuntime::register_legion_task< connect_mpi_task>( CONNECT_MPI_TASK_ID, 
                          Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "connect_mpi_task");
  
  HighLevelRuntime::register_legion_task< helloworld_mpi_task >( HELLOWORLD_TASK_ID,
                          Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "hellowrld_task");


  const InputArgs &args = HighLevelRuntime::get_input_args();

  HighLevelRuntime::start(args.argc, args.argv, true);

  complete_legion_configure();

  run_legion_task();  

  handshake->legion_handoff_to_ext();
}

#define execute(task, ...) \
  execution_t::execute_task(task, ##__VA_ARGS__)

TEST(mpi_with_legion, simple) {
   ASSERT_LT(execute(world_size), 1);

//   handshake = new ExtLegionHandshake(ExtLegionHandshake::IN_EXT, 1, 1);
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
