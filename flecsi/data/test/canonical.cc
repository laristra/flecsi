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
  constexpr static size_t value = 12;

  using entity_types = std::tuple<util::constant<base::vertices>,
    flecsi::util::constant<base::cells>>;

  static coloring color(std::string const &) {
    coloring c;
    return c;
  } // color
};

canon::slot canonical;
canon::cslot coloring;

const field<double>::definition<canon, canon::base::cells> cell_field;
auto pressure = cell_field(canonical);

int
check() {
  UNIT { flog(info) << "check" << std::endl; };
} // check

int
canonical_driver() {
  UNIT {
    const std::string filename = "input.txt";
    coloring.allocate(filename);
    canonical.allocate(coloring.get());

    EXPECT_EQ(test<check>(), 0);
  };
} // index

flecsi::unit::driver<canonical_driver> driver;
