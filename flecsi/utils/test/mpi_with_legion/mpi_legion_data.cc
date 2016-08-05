/*~--------------------------------------------------------------------------~*
 *
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

#include "flecsi/utils/mpi_legion_interoperability/mpi_legion_data.h"
//#include "flecsi/execution/mpilegion/execution_policy.h"
#include "flecsi/execution/task.h"
#include "legion.h"

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
};

using namespace flecsi;
using namespace flecsi::execution;


//make Array global only for the simple test example
//in general, we are not suppose to do so if the object
//is used in Legion

const int nElements=10;
MPILegionArray<double, nElements> Array;
std::vector<std::shared_ptr<MPILegionArrayStorage_t>> ArrayStorage;


void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, Runtime *runtime)
{
  std::cout << "Hello World Top Level Task" << std::endl;

  //*********** testing Array  

  Array.allocate_legion(ctx);
  Array.legion_init( ctx);
  Array.partition_legion(2,ctx);

//  Array.copy_mpi_to_legion(ctx);
 //this has to be done on the legion side 
  Array.copy_mpi_to_legion(ctx);

  Array.dump_legion("legion Array", 1, ctx);

  double init_value=14;
  Array.legion_init(init_value, ctx);
  //this has to be done on the legion side
  
  double *acc=Array.get_legion_accessor(WRITE_DISCARD, EXCLUSIVE, ctx);
  for (int i=0; i<nElements; i++)
   acc[i]=i;
  //we need to return accessor after calling "get_legion_accessor" to avoid deadlock
  Array.return_legion_accessor(ctx);

  Array.dump_legion("legion Array", 1, ctx);
 

  Array.copy_legion_to_mpi(ctx);

  Array.dump_mpi("  output for MPI Array ");
 
  Array.deallocate_legion(ctx);

  // ********* testing ArrayStorage
  for (uint i=0; i<ArrayStorage.size(); i++){
    ArrayStorage[i]->allocate_legion(ctx);
    ArrayStorage[i]->legion_init(ctx);
    ArrayStorage[i]->partition_legion(10,ctx);
    ArrayStorage[i]->copy_mpi_to_legion(ctx);
    ArrayStorage[i]->copy_legion_to_mpi(ctx);
    ArrayStorage[i]->deallocate_legion(ctx);
  }

}

void example_task(int beta,double alpha,element_t i,state_accessor_t<double> a, state_accessor_t<int> b)
{

}

//main test function
TEST(MPILegionArray, simple) {

  // ********* initialize mpi instance to 0
  Array.mpi_init();

  // ********** initialize with the value
  double init_value=13;
  Array.mpi_init(init_value);

  // ********** getting mpi accessor
  double *A1=Array.mpi_accessor();
  assert (A1[0]==13);  

  // ********* output an mpi instance
  Array.dump_mpi("  output for MPI Array ");
 
  for (int i=0; i< nElements; i++)
  {
    A1[i]=i*0.1;
  }

  Array.dump_mpi("  output for MPI Array ");

  int size=Array.size();
  assert (size=nElements);

  // ********* check for ArrayStorage
  MPILegionArray<double, nElements> *ArrayDouble = new  MPILegionArray<double, nElements>();
  MPILegionArray<int, nElements-4> *ArrayInt = new MPILegionArray<int, nElements-4 >();
  MPILegionArray <uint, 10> *ArrayUint = new MPILegionArray<uint, 10>();

  ArrayStorage.push_back(std::shared_ptr<MPILegionArrayStorage_t>(ArrayDouble));
  ArrayStorage.push_back(std::shared_ptr<MPILegionArrayStorage_t>(ArrayInt));
  ArrayStorage.push_back(std::shared_ptr<MPILegionArrayStorage_t>(ArrayUint));

  for (uint i=0; i<ArrayStorage.size(); i++){
    ArrayStorage[i]->mpi_init();
  }

  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  HighLevelRuntime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/);


  const InputArgs &args = HighLevelRuntime::get_input_args();

  HighLevelRuntime::start(args.argc, args.argv);

  //we need to call delete on MPILegion arrays here, but we can't do this because execute_driver 
  //is non-blocking operation. Please see mpi_legion_interop for the correct implementation

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
