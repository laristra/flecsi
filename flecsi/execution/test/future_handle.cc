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

template<typename T>
using future_handle_t =
  flecsi::execution::flecsi_future<T, flecsi::execution::launch_type_t::single>;

template<typename T>
using index_handle_t =
  flecsi::execution::flecsi_future<T, flecsi::execution::launch_type_t::index>;

void
future_dump(future_handle_t<double> x) {
  double tmp = x.get();
  std::cout << " future = " << x.get() << std::endl;
}

flecsi_register_task(future_dump, , loc, single);

double
writer(double a) {
  double x = 3.14;
  return x;
}

flecsi_register_task(writer, , loc, single);

void
reader(future_handle_t<double> x, future_handle_t<double> y) {
  ASSERT_EQ(x.get(), static_cast<double>(3.14));
  ASSERT_EQ(x.get(), y.get());
}

flecsi_register_task(reader, , loc, single);

int
index_writer() {
  int x =
    1 + static_cast<int>(flecsi::execution::context_t::instance().color());
  return x;
}

flecsi_register_task(index_writer, , loc, index);

void
index_reader(index_handle_t<int> x) {
  int y =
    1 + static_cast<int>(flecsi::execution::context_t::instance().color());
  ASSERT_EQ(x.get(), y);
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
