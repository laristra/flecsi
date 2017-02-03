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

#include "flecsi/execution/execution.h"
#include "flecsi/execution/task.h"
#include "flecsi/utils/any.h"
#include "flecsi/partition/index_partition.h"

using namespace flecsi::execution;

enum TaskIDs{
 TOP_LEVEL_TASK_ID         =0x00000010,
 HELLOWORLD_TASK_ID        =0x00000100,
 SPMD_INIT_TID             =0x00000200,
};



//make Array global only for the simple test example
//in general, we are not suppose to do so if the object
//is used in Legion

const int nElements=10;

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;
using index_partition_t = flecsi::dmp::index_partition__<size_t>;

static mpi_legion_interop_t InteropHelper;

/* ------------------------------------------------------------------------- */
 void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, Runtime *runtime)
 {
  InteropHelper.connect_with_mpi(ctx, runtime);

  std::cout<<"inside TLT:after connect_with_mpi"<<std::endl;

  InteropHelper.wait_on_mpi(ctx, runtime);
  ArgumentMap arg_map;
  IndexLauncher helloworld_launcher(
       HELLOWORLD_TASK_ID,
       Domain::from_rect<1>(
              InteropHelper.all_processes_),
       TaskArgument(0, 0),
       arg_map);

  FutureMap fm2 =
     runtime->execute_index_space(ctx, helloworld_launcher);
  fm2.wait_all_results();

  //handoff to MPI
  InteropHelper.handoff_to_mpi(ctx, runtime);

  InteropHelper.wait_on_mpi(ctx, runtime);

  std::cout<< "back to TLT after MPI" <<std::endl;

  //check array storage 

   index_partition_t ip = InteropHelper.data_storage_[0];
   double A= InteropHelper.data_storage_[1];
   std::cout<<"storage[1] = " << A <<std::endl;
   assert (A==3.14);
 
  InteropHelper.handoff_to_mpi(ctx, runtime);

   
 }

/* ------------------------------------------------------------------------- */
void helloworld_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
  printf ("helloworld \n");
}


/* ------------------------------------------------------------------------- */
void my_init_legion(){
  
  InteropHelper.legion_configure();

  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  HighLevelRuntime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/);

  HighLevelRuntime::register_legion_task< helloworld_mpi_task >(
        HELLOWORLD_TASK_ID,
        Processor::LOC_PROC,
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        TaskConfigOptions(true/*leaf*/),
        "hellowrld_task");

     InteropHelper.register_tasks();
     InteropHelper.initialize();

  char arguments[] = "1";
  char * argv = &arguments[0];
  HighLevelRuntime::start(1, &argv, true);


  InteropHelper.handoff_to_legion();

  InteropHelper.wait_on_legion();

  //check data_storage_
  index_partition_t ip;
  double A=3.14;
  InteropHelper.data_storage_.push_back(flecsi::utils::any_t(ip));
  InteropHelper.data_storage_.push_back(flecsi::utils::any_t(A));

  InteropHelper.handoff_to_legion();
 
  InteropHelper.wait_on_legion();
  Legion::Runtime::wait_for_shutdown();
  std::cout<<"back to MPI to finalize"<<std::endl;
}

/* ------------------------------------------------------------------------- */
TEST(mpi_legion_interop, simple) {
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
