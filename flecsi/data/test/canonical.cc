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
#include "flecsi/topo/canonical/interface.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

#include <tuple>

using namespace flecsi;

struct canon : topo::specialization<topo::canonical, canon> {
  enum index_space { vertices, cells };
  using index_spaces = has<cells, vertices>;
  using connectivities = util::types<from<cells, has<vertices>>>;

  static coloring color(std::string const &) {
    return {{16, 17}, 2};
  } // color
};

canon::slot canonical;
canon::cslot coloring;

const field<double>::definition<canon, canon::cells> cell_field;
auto pressure = cell_field(canonical);

const int mine = 35;
const util::id favorite = 3;
const double p0 = 3.5;

void
allocate0(topo::resize::Field::accessor<wo> a) {
  a = data::partition::make_row(color(), 1 + 2 + 3 + 4);
}
void
allocate(field<util::id, data::ragged>::mutator m) {
  for(int i = 0; i < 4; ++i)
    m[i].resize(i + 1);
}

int
init(canon::accessor<wo> t, field<double>::accessor<wo> c) {
  UNIT {
    t.mine(0) = mine;
    t.get_connect<canon::cells, canon::vertices>()[3].back() = favorite;
    c(0) = p0;
  };
} // init

// Exercise the std::vector-like interface:
int
permute(field<util::id, data::ragged>::mutator m) {
  UNIT {
    const auto &&src = m[3], &&dst = m[0], &&two = m[1];
    // Intermediate sizes can exceed the capacity of the underlying raw field:
    dst.insert(dst.begin(), 10, 3);
    EXPECT_EQ(dst.end()[-1], 0u);
    EXPECT_EQ(dst.end()[-2], 3u);
    src.erase(src.begin(), src.end() - 1); // keep the one real value
    src.insert(src.begin(), dst.begin(), dst.end());
    two.resize(3);
    ASSERT_EQ(two.size(), 3u);
    EXPECT_GT(two.size(), two.capacity());
    ASSERT_EQ(&two[0] + 1, &two[1]);
    EXPECT_NE(&two[1] + 1, &two[2]); // the latter is in the overflow
    two.erase(two.begin() + 1);
    EXPECT_NE(&two[0] + 1, &two[1]); // [1] now refers to the overflow
    two.pop_back();
    EXPECT_EQ(two.size(), 1u);
    two.push_back(0);
    EXPECT_EQ(&two[0] + 1, &two[1]); // TODO: test shrink_to_fit
    dst.clear();
    two.clear();
    dst.push_back(src.back());
    src.clear();
  };
}

int
check(canon::accessor<ro> t, field<double>::accessor<ro> c) {
  UNIT {
    auto & r = t.mine(0);
    static_assert(std::is_same_v<decltype(r), const int &>);
    EXPECT_EQ(r, mine);
    auto & cv = t.get_connect<canon::cells, canon::vertices>()[0].front();
    static_assert(std::is_same_v<decltype(cv), const util::id &>);
    EXPECT_EQ(cv, favorite);
    EXPECT_EQ(c(0), p0);
  };
} // check

// Making the partition wider would require initializing the new elements.
void
shrink(topo::resize::Field::accessor<rw> a) {
  a = data::partition::make_row(color(), data::partition::row_size(a) - 1);
}

int
canonical_driver() {
  UNIT {
    const std::string filename = "input.txt";
    coloring.allocate(filename);
    canonical.allocate(coloring.get());

    auto & cf = canonical->connect.get<canon::cells>().get<canon::vertices>();
    auto & p = canonical->ragged->get_partition<canon::cells>(cf.fid);
    execute<allocate0>(p.sizes());
    p.resize();
    execute<allocate>(cf(canonical));
    EXPECT_EQ(test<init>(canonical, pressure), 0);
    EXPECT_EQ(test<permute>(cf(canonical)), 0);
    EXPECT_EQ(test<check>(canonical, pressure), 0);

    auto & c = canonical.get().part.get<canon::cells>();
    execute<shrink>(c.sizes());
    c.resize();
    EXPECT_EQ(test<check>(canonical, pressure), 0);
  };
} // index

flecsi::unit::driver<canonical_driver> driver;
