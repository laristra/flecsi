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

struct policy {
  constexpr static size_t value = 12;

  enum index_space { vertices, cells };
  static constexpr std::size_t index_spaces = 2;

  using entity_types =
    std::tuple<util::constant<vertices>, flecsi::util::constant<cells>>;

  using coloring = topo::canonical_base::coloring;

  static coloring color(std::string const &) {
    coloring c;
    return c;
  } // color

}; // struct policy

using topology_type = topo::canonical<policy>;

using canonical_topology = data::topology_slot<topology_type>;
canonical_topology canonical;

data::coloring_slot<topology_type> coloring;

const field<double>::definition<topology_type, policy::cells> cell_field;
auto pressure = cell_field(canonical);

int
check() {
  UNIT {
    flog(info) << "check" << std::endl;
  };
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
