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
using intN = field<int, ragged>;
const intN::definition<topo::index> verts_field;

const auto pressure = pressure_field(process_topology);
const auto verts = verts_field(process_topology);

void
allocate(resize::Field::accessor<wo> a) {
  const auto i = color();
  a = partition::make_row(i, i + 1);
}
void
rows(intN::Offsets::accessor<wo> a) {
  static_assert(std::is_same_v<decltype(a(0)), std::size_t &>);
  a(0) = color() + 1;
}

using noisy = field<Noisy, singular>;
const noisy::definition<topo::index> noisy_field;
const auto noise = noisy_field(process_topology);

void
assign(double1::accessor<wo> p, intN::accessor<wo> r) {
  const auto i = color();
  flog(info) << "assign on " << i << std::endl;
  p = i;
  static_assert(std::is_same_v<decltype(r.get_offsets().span()),
    util::span<const std::size_t>>);
  const auto s = r[0];
  for(auto & x : s)
    x = 0;
  s.back() = 1;
} // assign

std::size_t
reset(noisy::accessor<wo>) {
  return Noisy::count;
}

int
check(double1::accessor<ro> p, intN::accessor<ro> r, noisy::accessor<ro> n) {
  UNIT {
    const auto me = color();
    flog(info) << "check on " << me << std::endl;
    ASSERT_EQ(p, me);
    ASSERT_EQ(r.size(), 1u);
    const auto s = r[0];
    static_assert(std::is_same_v<decltype(s), const util::span<const int>>);
    ASSERT_EQ(s.size(), me + 1);
    EXPECT_EQ(s.back(), 1);
    EXPECT_EQ(n.get().i, Noisy::value());
  };
} // print

int
index_driver() {
  UNIT {
    Noisy::count = 0;
    auto & p = process_topology.get().ragged->get_partition<topo::elements>(
      verts_field.fid);
    execute<allocate>(p.sizes());
    p.resize();
    execute<rows>(verts_field.offsets(process_topology));
    execute<assign>(pressure, verts);
    execute<reset>(noise);
    EXPECT_EQ((reduce<reset, exec::fold::sum<std::size_t>, mpi>(noise).get()),
      processes());
    EXPECT_EQ(test<check>(pressure, verts, noise), 0);
  };
} // index

flecsi::unit::driver<index_driver> driver;
