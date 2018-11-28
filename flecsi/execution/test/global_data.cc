/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2018, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

///
/// \file
/// \date Initial file creation: Apr 3, 2018
///

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/data/global_accessor.h>

using namespace flecsi;

template<size_t PERMISSION>
using gint = global_accessor_u<int, PERMISSION>;

void set_global_int(gint<rw> global, int value) {
  auto& context = execution::context_t::instance();
  auto rank = context.color();
  std::cout << "[" << rank << "] setting value" << std::endl;
  global = value;
}

void check_global_int(gint<ro> global, int value) {
  auto& context = execution::context_t::instance();
  auto rank = context.color();
  ASSERT_EQ(global, static_cast<int>(value));
}

void hello_world() {
  auto& context = execution::context_t::instance();
  auto rank = context.color();
  std::cout << "Hello world from rank " << rank << std::endl;
}

flecsi_register_task_simple(set_global_int, loc, single);
flecsi_register_task_simple(check_global_int, loc, index);
flecsi_register_task_simple(hello_world, loc, index);

flecsi_register_global(global, int1, int, 1);
flecsi_register_global(global, int2, int, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  auto gh0 = flecsi_get_global(global, int1, int, 0);
  auto gh1 = flecsi_get_global(global, int2, int, 0);

  // rank 0
  flecsi_execute_task_simple(set_global_int, single, gh0, 42);

  // all ranks
   flecsi_execute_task_simple(hello_world, index);

  // rank 0
   flecsi_execute_task_simple(set_global_int, single, gh1, 2042);

   flecsi_execute_task_simple(check_global_int, index, gh0, 42);
   flecsi_execute_task_simple(check_global_int, index, gh1, 2042);
   flecsi_execute_task_simple(hello_world, index);

} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
  auto gh0 = flecsi_get_global(global, int1, int, 0);
  auto gh1 = flecsi_get_global(global, int2, int, 0);

  // not allowed
//   flecsi_execute_task_simple(set_global_int, index, gh0, 43);

  flecsi_execute_task_simple(check_global_int, index, gh0, 42);
  flecsi_execute_task_simple(check_global_int, index, gh1,2042);

  flecsi_execute_task_simple(hello_world, index);

} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  auto gh0 = flecsi_get_global(global, int1, int, 0);
  auto gh1 = flecsi_get_global(global, int2, int, 0);

  flecsi_execute_task_simple(check_global_int, index, gh0, 42);
  flecsi_execute_task_simple(check_global_int, index, gh1, 2042);

  // flecsi_execute_task_simple(hello_world, index);

  // auto& context = execution::context_t::instance();
  // if(context.color() == 0){
  //   ASSERT_TRUE(CINCH_EQUAL_BLESSED("global_data.blessed"));
  // }

} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(dense_data, testname) {

} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
