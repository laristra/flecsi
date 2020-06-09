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

#include "flecsi/data.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/execution.hh>

using namespace flecsi;

using double1 = field<double, data::singular>;
const double1::definition<topo::global> energy_field;

const auto energy = energy_field(global_topology);

namespace future_test {

double
init(double a, double1::accessor<wo> ga) {
  ga = a;
  return a + 1;
}

int
check(future<double> x, double1::accessor<ro> ga) {
  UNIT {
    static_assert(std::is_same_v<decltype(ga.get()), const double &>);
    ASSERT_EQ(x.get(), ga + 1 + run::context::instance().color());
  };
}

double
index_init(double a, exec::launch_domain) {

  auto & c = run::context::instance();
  return a + c.color();
}

void
void_task() {
  flog(info) << "this is a void task" << std::endl;
}

void
index_void_task(exec::launch_domain) {
  flog(info) << "this is an index void task" << std::endl;
}
} // namespace future_test

int
future_driver() {
  UNIT {
    using namespace future_test;

    // single future
    auto f = execute<init>(3.1, energy);

    EXPECT_EQ(test<check>(f, energy), 0);
    EXPECT_EQ(f.get(), 3.1 + 1);

    // future map
    const exec::launch_domain ld{run::context::instance().processes()};
    auto fm = execute<index_init>(f.get(), ld);
    EXPECT_EQ(fm.get(0, false), f.get());

    // For all values because it's an index future:
    EXPECT_EQ(test<check>(fm, energy), 0);

    auto fv = execute<void_task>();

    fv.wait();
    fv.get();

    auto fv2 = execute<index_void_task>(ld);

    fv2.wait();
    fv2.get();
  };
} // future

flecsi::unit::driver<future_driver> driver;
