/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_task_driver_h
#define flecsi_task_driver_h

#include <cinchtest.h>
#include <iostream>

#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/utils/common.h>

//----------------------------------------------------------------------------//
// Task registration.
//----------------------------------------------------------------------------//

using array_t = std::array<double, 2>;

template<typename T>
using handle_t =
  flecsi::execution::flecsi_future<T, flecsi::execution::launch_type_t::single>;

template<typename T>
using index_handle_t =
  flecsi::execution::flecsi_future<T, flecsi::execution::launch_type_t::index>;

void
future_dump(handle_t<double> x) {
  double tmp = x;
  std::cout << " future = " << x << std::endl;
}

flecsi_register_task(future_dump, , loc, single);

double
writer(double a) {
  double x = 3.14;
  return x;
}

array_t
array_writer(double a) {
  return {3.14, 2 * 3.14};
}

flecsi_register_task(writer, , loc, single);
flecsi_register_task(array_writer, , loc, single);

void
reader(handle_t<double> x, handle_t<double> y) {
  ASSERT_EQ(x, static_cast<double>(3.14));
  ASSERT_EQ(x, y);
}

void
array_reader(handle_t<array_t> x) {
  ASSERT_EQ(x.get()[0], static_cast<double>(3.14));
  ASSERT_EQ(x.get()[1], static_cast<double>(2 * 3.14));
}

flecsi_register_task(reader, , loc, single);
flecsi_register_task(array_reader, , loc, single);

int
index_writer() {
  int x = 1 + flecsi::execution::context_t::instance().color();
  return x;
}

flecsi_register_task(index_writer, , loc, index);

void
index_reader(index_handle_t<int> x) {
  int y = 1 + flecsi::execution::context_t::instance().color();
  ASSERT_EQ(x, y);
}

flecsi_register_task(index_reader, , loc, index);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

  auto & context = execution::context_t::instance();
  ASSERT_EQ(
    context.execution_state(), static_cast<size_t>(SPECIALIZATION_TLT_INIT));
}
//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto & context = execution::context_t::instance();
  ASSERT_EQ(context.execution_state(), static_cast<size_t>(DRIVER));

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto future = flecsi_execute_task(writer, , single, 0.0);
  flecsi_execute_task(future_dump, , single, future);
  flecsi_execute_task(reader, , single, future, future);
  
  auto array_future = flecsi_execute_task(array_writer, , single, 0.0);
  flecsi_execute_task(array_reader, , single, array_future);

  auto fm = flecsi_execute_task(index_writer, , index);
  flecsi_execute_task(index_reader, , index, fm);

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(simple_task, testname) {} // TEST

} // namespace execution
} // namespace flecsi

#endif // flecsi_task_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
