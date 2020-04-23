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

  enum index_space { vertices, cells };
  static constexpr std::size_t index_spaces = 2;

  using entity_types =
    std::tuple<util::constant<vertices>, flecsi::util::constant<cells>>;

  static coloring color(std::string const &) {
    coloring c;
    return c;
  } // color
};

canon::slot canonical;
canon::cslot coloring;

const field<double>::definition<canon, canon::cells> cell_field;
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
