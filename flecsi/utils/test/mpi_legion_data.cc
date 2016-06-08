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
#include "flecsi/execution/mpilegion_execution_policy.h"
#include "flecsi/execution/legion_execution_policy.h"
#include "flecsi/execution/task.h"
#include "legion.h"

#include "flecsi/utils/TaskWrapper.h"

#include "flecsi/execution/register_legion.h"

using namespace flecsi;
using namespace flecsi::mpilegion;

using execution_type = execution_t<mpilegion_execution_policy_t>;
using return_type_t = execution_type::return_type_t;

//make Array global only for the simple test example
//in general, we are not suppose to do so if the object
//is used in Legion

const int nElements=10;
MPILegionArray<double, nElements> Array;

typedef typename flecsi::context_t<flecsi::mpilegion_execution_policy_t> mpilegion_context;
namespace flecsi
{
void mpilegion_top_level_task(mpilegion_context &&ctx,int argc, char** argv)
{
  std::cout << "Hello World Top Level Task" << std::endl;
  //A.allocate_legion
  
}
}

void example_task(int beta,double alpha,element_t i,state_accessor_t<double> a, state_accessor_t<int> b)
{

}

//main test function
TEST(mpi_legion_interop_and_data, sanity) {

  double *A1=Array.mpi_accessor();  
  std::cout << A1[0] << std::endl;

  using wrapper_t = TaskWrapper<1,0,0,0,legion_execution_policy_t,std::function<decltype(example_task)>>;
  //register_legion<wrapper_t>::register_task();

  char d[] = "something";
  char *argv = &(d[0]);
  execution_type::execute_driver(flecsi::mpilegion_top_level_task,1,&argv);

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
