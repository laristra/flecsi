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

  static coloring color(std::string const &) {
    return {{16, 17}, 2};
  } // color
};

canon::slot canonical;
canon::cslot coloring;

const field<double>::definition<canon, canon::cells> cell_field;
auto pressure = cell_field(canonical);

const int mine = 35;
const double p0 = 3.5;

int
init(canon::accessor<wo> t, field<double>::accessor<wo> c) {
  UNIT {
    t.mine(0) = mine;
    c(0) = p0;
  };
} // init

int
check(canon::accessor<ro> t, field<double>::accessor<ro> c) {
  UNIT {
    auto & r = t.mine(0);
    static_assert(std::is_same_v<decltype(r), const int &>);
    EXPECT_EQ(r, mine);
    EXPECT_EQ(c(0), p0);
  };
} // check

int
canonical_driver() {
  UNIT {
    const std::string filename = "input.txt";
    coloring.allocate(filename);
    canonical.allocate(coloring.get());

    EXPECT_EQ(test<init>(canonical, pressure), 0);
    EXPECT_EQ(test<check>(canonical, pressure), 0);
  };
} // index

flecsi::unit::driver<canonical_driver> driver;
