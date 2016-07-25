/*~-------------------------------------------------------------------------~~*
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
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/execution/legion_execution_policy.h"
#include "flecsi/execution/task.h"
#include "flecsi/utils/TaskWrapper.h"

#include "flecsi/execution/register_legion.h"

using namespace flecsi;
using execution_t = flecsi::execution_t<flecsi::legion_execution_policy_t>;
using return_type_t = flecsi::execution_t<flecsi::legion_execution_policy_t>::return_type_t;


void example_task(int beta,double alpha,element_t i,state_accessor_t<double> a, state_accessor_t<int> b)
{

}

return_type_t testme(const char * token) {
  std::cout << "Hello World: " << token << std::endl;
  return 0;
} // testme

return_type_t myvoid() {
  std::cout << "Hello World: " << std::endl;
  return 0;
}

namespace flecsi{
void top_level_task(context_t<flecsi::legion_execution_policy_t> &&ctx,int argc, char** argv)
{
	std::cout << "Hello World Top Level Task" << std::endl;
}
}
#define execute(task, ...) \
  execution_t::execute_task(task, ##__VA_ARGS__)

TEST(legion, register_task) {
  using wrapper_t = TaskWrapper<1,0,0,0,legion_execution_policy_t,std::function<decltype(example_task)>>;

  register_legion<wrapper_t>::register_task();


} // TEST

//TEST(task, execute) {
//  execute(testme, "shit");
//  execute(myvoid);
//} // TEST

TEST(driver, execute) {
	char dummy[] = "this is a test";
	char *argv = &(dummy[0]);
	flecsi::execution_t<flecsi::legion_execution_policy_t>::execute_driver(flecsi::top_level_task,1,&argv);
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

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
