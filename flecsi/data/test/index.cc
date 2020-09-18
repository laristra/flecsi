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
using double_at = field<double, sparse>;
const double_at::definition<topo::index> vfrac_field;

constexpr std::size_t column = 42;

void
allocate(resize::Field::accessor<wo> a) {
  const auto i = color();
  a = partition::make_row(i, i + 1);
}
void
rows(intN::mutator r) {
  r[0].resize(color() + 1);
}
int
drows(double_at::mutator s) {
  UNIT {
    const auto me = color();
    const auto && m = s[0];
    for(std::size_t c = 0; c <= me; ++c)
      m.try_emplace(column + c, me + c);
    for(const auto && p : m)
      EXPECT_EQ(p.first - column, p.second - me);
    // Exercise std::map-like interface:
    {
      const auto [i, nu] = m.insert({column, 0});
      EXPECT_FALSE(nu);
      EXPECT_EQ((*i).first, column);
      EXPECT_EQ((*i).second, me);
    }
    {
      const auto [i, nu] = m.try_emplace(0, 1);
      EXPECT_TRUE(nu);
      EXPECT_EQ((*i).first, 0u);
      EXPECT_EQ((*i).second, 1u);
    }
    {
      const auto [i, nu] = m.insert_or_assign(0, 2);
      EXPECT_FALSE(nu);
      EXPECT_EQ((*i).first, 0u);
      EXPECT_EQ((*i).second, 2u);
    }
    EXPECT_EQ(m.count(0), 1u);
    EXPECT_EQ(m.erase(0), 1u);
    EXPECT_EQ(m.count(0), 0u);
    EXPECT_EQ(m.erase(0), 0u);
    EXPECT_EQ((*m.find(column)).second, me);
    EXPECT_EQ(m.lower_bound(column), m.begin());
    EXPECT_EQ(m.upper_bound((*--m.end()).first), m.end());
  };
}

using noisy = field<Noisy, singular>;
const noisy::definition<topo::index> noisy_field;

void
assign(double1::accessor<wo> p,
  intN::accessor<rw> r,
  double_at::accessor<rw> sp) {
  const auto i = color();
  flog(info) << "assign on " << i << std::endl;
  p = i;
  static_assert(std::is_same_v<decltype(r.get_offsets().span()),
    util::span<const std::size_t>>);
  r[0].back() = 1;
  ++sp[0](column + i);
} // assign

std::size_t reset(noisy::accessor<wo>) { // must be an MPI task
  return Noisy::count;
}

int
check(double1::accessor<ro> p,
  intN::accessor<ro> r,
  double_at::accessor<ro> sp,
  noisy::accessor<ro> n) {
  UNIT {
    const auto me = color();
    flog(info) << "check on " << me << std::endl;
    ASSERT_EQ(p, me);
    ASSERT_EQ(r.size(), 1u);
    const auto s = r[0];
    static_assert(std::is_same_v<decltype(s), const util::span<const int>>);
    ASSERT_EQ(s.size(), me + 1);
    EXPECT_EQ(s.back(), 1);
    ASSERT_EQ(sp.size(), 1u);
    const auto sr = sp[0];
    EXPECT_EQ(sr(column + me), 2 * me + 1);
    EXPECT_EQ(n.get().i, Noisy::value());
  };
} // print

int
index_driver() {
  UNIT {
    Noisy::count = 0;
    for(const auto f : {verts_field.fid, vfrac_field.fid}) {
      auto & p = process_topology->ragged.get_partition<topo::elements>(f);
      execute<allocate>(p.sizes());
      p.resize();
    }
    const auto pressure = pressure_field(process_topology);
    const auto verts = verts_field(process_topology);
    const auto vfrac = vfrac_field(process_topology);
    const auto noise = noisy_field(process_topology);
    execute<rows>(verts);
    execute<drows>(vfrac);
    execute<assign>(pressure, verts, vfrac);
    execute<reset>(noise);
    EXPECT_EQ((reduce<reset, exec::fold::sum<std::size_t>, mpi>(noise).get()),
      processes());
    EXPECT_EQ(test<check>(pressure, verts, vfrac, noise), 0);
  };
} // index

flecsi::unit::driver<index_driver> driver;
