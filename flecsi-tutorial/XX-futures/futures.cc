/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <cstdlib>
#include <iostream>

#include<flecsi/execution/execution.h>

using namespace flecsi;


namespace tasks {

template<typename T>
using handle_t = flecsi::execution::flecsi_future<T,
    flecsi::execution::launch_type_t::single>;

void future_dump(handle_t<double> x)
{
  double tmp = x;
  std::cout << " future = "<< x << std::endl;
}

flecsi_register_task(future_dump, tasks, loc, single);

double writer(double a)
{
  double x = 3.14;
  return x;
}

flecsi_register_task(writer, tasks,loc, single);

void reader(handle_t<double> x, handle_t<double> y)
{
  assert(x==static_cast<double>(3.14));
  assert(x==y);
}

flecsi_register_task(reader, tasks, loc, single);

} // namespace tasks

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto future = flecsi_execute_task( writer, tasks, single, 0.0);
  flecsi_execute_task( future_dump, tasks, single, future);
  flecsi_execute_task( reader, tasks, single, future, future);

} // driver

} // namespace execution
} // namespace flecsi
