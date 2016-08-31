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

static mpi_legion_interop_t InteropHelper;
ext_legion_handshake_t &handshake=ext_legion_handshake_t::instance(); 

void print_task(void)
{
  std::cout<<"print_task from MPI"<<std::endl;
}

/* ------------------------------------------------------------------------- */
 void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, Runtime *runtime)
 {
  InteropHelper.connect_with_mpi(ctx, runtime);

  std::cout<<"inside TLT:after connect_with_mpi"<<std::endl;

  ArgumentMap arg_map;
  IndexLauncher helloworld_launcher(
       HELLOWORLD_TASK_ID,
       Domain::from_rect<1>(
              InteropHelper.local_procs_),
       TaskArgument(0, 0),
       arg_map);
  //TOFIX:: add checkfor compy data functions
  // + for passing function pointer between MPI and Legion

  FutureMap fm2 =
     runtime->execute_index_space(ctx, helloworld_launcher);
  fm2.wait_all_results();
  //handoff to MPI

  InteropHelper.shared_func_=std::bind(print_task);  
  InteropHelper.call_mpi_=true;

  InteropHelper.handoff_to_mpi(ctx, runtime);

  InteropHelper.wait_on_mpi(ctx, runtime);

  std::cout<< "back to TLT after MPI" <<std::endl;
  
   InteropHelper.call_mpi_=false;

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

   handshake.initialize(ext_legion_handshake_t::IN_EXT,1,1);
     InteropHelper.register_tasks();
     InteropHelper.initialize();

  char arguments[] = "1";
  char * argv = &arguments[0];

  HighLevelRuntime::start(1, &argv, true);

  std::cout<<"before legion_configure" <<std::endl;
  InteropHelper.legion_configure();

  std::cout<<"before handoff_to_legion" <<std::endl;
  InteropHelper.handoff_to_legion();

  InteropHelper.wait_on_legion();

  while(InteropHelper.call_mpi_)
     {
       InteropHelper.shared_func_();
       InteropHelper.handoff_to_legion();
       InteropHelper.wait_on_legion();
     }

 //check data_storage_
 //TOFIX:
// using index_partition_t = dmp::index_partition__<size_t>;
// array__<index_partition_t,5> *array=new array__<index_partition_t,5>();
// InteropHelper.data_storage_.push_back(
//           std::shared_ptr<mpi_array_storage_t>(array));

 InteropHelper.wait_on_legion();
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
