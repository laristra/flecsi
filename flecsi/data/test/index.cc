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

#define __FLECSI_PRIVATE__
#include "flecsi/util/demangle.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topo;

struct Noisy {
  ~Noisy() {
    ++count;
  }
  std::size_t i = value();
  static std::size_t value() {
    return color() + 1;
  }
  static inline std::size_t count;
};

using double1 = field<double, singular>;
const double1::definition<topo::index> pressure_field;

const auto pressure = pressure_field(process_topology);

using noisy = field<Noisy, singular>;
const noisy::definition<topo::index> noisy_field;
const auto noise = noisy_field(process_topology);

void
assign(double1::accessor<wo> p) {
  const auto i = process();
  flog(info) << "assign on " << i << std::endl;
  p = i;
} // assign

std::size_t
reset(noisy::accessor<wo>) {
  return Noisy::count;
}

int
check(double1::accessor<ro> p, noisy::accessor<ro> n) {
  UNIT {
    flog(info) << "check on " << color() << std::endl;
    ASSERT_EQ(p, color());
    EXPECT_EQ(n.get().i, Noisy::value());
  };
} // print

int
index_driver() {
  UNIT {
    Noisy::count = 0;
    execute<assign>(pressure);
    execute<reset>(noise);
    EXPECT_EQ((reduce<reset, exec::fold::sum<std::size_t>, mpi>(noise).get()),
      processes());
    EXPECT_EQ(test<check>(pressure, noise), 0);
  };
} // index

flecsi::unit::driver<index_driver> driver;
