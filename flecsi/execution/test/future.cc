/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/utils/ftest.hh>

#define __FLECSI_PRIVATE__
#include <flecsi/execution.hh>

flog_register_tag(future);

using namespace flecsi;

namespace future_test {

template<typename T>
using handle_t = flecsi::execution::flecsi_future<T>;

double
init(double a) {
  double x = a;
  return x;
}

int
read(handle_t<double> x, handle_t<double> y) {
  FTEST();
  auto x1 = x.get();
  auto y1 = 2 * y.get();

  ASSERT_EQ(x1, y1);

  return FTEST_RESULT();
}

double
index_init(double a) {

  auto & c = flecsi::runtime::context_t::instance();
  auto process = c.process();
  double x = process * a;
  return x;
}

int
index_read(handle_t<double> x, handle_t<double> y) {
  FTEST();
  auto x1 = x.get();
  auto y1 = 2 * y.get();

  ASSERT_EQ(x1, y1);

  return FTEST_RESULT();
}

void
void_task() {
  std::cout << "this is a void task" << std::endl;
}

void
index_void_task() {
  std::cout << "this is an index void task" << std::endl;
}
} // namespace future_test

int
test_driver(int, char **) {

  FTEST();

  using namespace flecsi;
  using namespace flecsi::execution;
  // single future
  auto f1 = execute<future_test::init, single>(6.2);
  auto f2 = execute<future_test::init, single>(3.1);
  execute<future_test::read, single>(f1, f2);
  f1 = execute<future_test::init, single>(8.2);
  f2 = execute<future_test::init, single>(4.1);

  ASSERT_EQ(f1.get(), (2 * f2.get()));

  execute<future_test::read, single>(f1, f2);
  execute<future_test::read>(f1, f2);

  // future map
  auto fm1 = execute<future_test::index_init>(1.4);
  auto fm2 = execute<future_test::index_init>(0.7);
  execute<future_test::index_read>(fm1, fm2);

  ASSERT_EQ(fm1.get(0), (2 * fm2.get(0)));

  auto fv = execute<future_test::void_task, single>();
  fv.wait();
  fv.get();

  auto fv2 = execute<future_test::index_void_task>();
  fv2.wait();
  fv2.get();

  return FTEST_RESULT();
}

ftest_register_driver(test_driver);
