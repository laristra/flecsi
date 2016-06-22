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

#include <string>

#include <cinchtest.h>

//do not include mpi_legion_interop.h explicitly, it is included in 
//mpilegion_execution_policy.h to avoid circular dependency
//#include "flecsi/utils/mpi_legion_interoperability/mpi_legion_interop.h"
#include "flecsi/execution/mpilegion_execution_policy.h"
#include "flecsi/execution/task.h"

enum TaskIDs{
 HELLOWORLD_TASK_ID        =0x00000100,
};

using namespace flecsi;

using execution_type = execution_t<flecsi::mpilegion_execution_policy_t>;
using return_type_t = execution_type::return_type_t;


typedef typename flecsi::context_t<flecsi::mpilegion_execution_policy_t> mpilegion_context;
namespace flecsi
{
void mpilegion_top_level_task(mpilegion_context &&ctx,int argc, char** argv)
{
 MPILegionInteropHelper->allocate_legion(ctx);
 MPILegionInteropHelper->legion_init(ctx);
 MPILegionInteropHelper->copy_data_from_mpi_to_legion(ctx);
 //do some stuff on the data
 MPILegionInteropHelper->copy_data_from_legion_to_mpi(ctx);
}
}


void helloworld_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
  printf ("helloworld \n");
}


TEST(mpi_legion_interop_and_data, sanity) {

  //required to be the firs function call. 
  //TOFIX:: needs to be a part of the driver 
  MPILegion_Init();

  //TOFIX:: needs to be replaced with flecsi's reister task when it is fixed
  HighLevelRuntime::register_legion_task< helloworld_mpi_task >( HELLOWORLD_TASK_ID,
                          Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "hellowrld_task");

  

  const int nElements=10;
  MPILegionArray<double, nElements> *ArrayDouble= new MPILegionArray<double, nElements>;
  MPILegionArray<int, nElements> *ArrayInt= new MPILegionArray<int, nElements>;
  MPILegionArray<double, nElements> *ArrayResult= new MPILegionArray<double, nElements>;

  MPILegionInteropHelper->add_array_to_storage(ArrayDouble);
  MPILegionInteropHelper->add_array_to_storage(ArrayInt);
  MPILegionInteropHelper->add_array_to_storage(ArrayResult);

  assert (MPILegionInteropHelper->storage_size()==3);

  //zeroing all mpi instances pushed to MPILegionInteropHelper's storage
  MPILegionInteropHelper->mpi_init();

  ArrayDouble->mpi_init(1.1);
  ArrayInt->mpi_init(4);
  int *AInt = ArrayInt->mpi_accessor();
  double *ADouble = ArrayDouble->mpi_accessor();
  double *AResult=ArrayResult->mpi_accessor();
  for (int i=0; i< nElements; i++)
  {
    AResult[i]=AInt[i]*ADouble[i];
  }

  assert(AResult[0]==4.4);
  

 
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
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
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
