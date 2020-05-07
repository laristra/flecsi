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
#include "flecsi/topo/structured/interface.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;

struct policy {

  using coloring = topo::structured_base::coloring;

  static coloring color() {
    coloring c = 10;
    return c;
  } // color

}; // struct policy

using topology_type = topo::structured<policy>;

using structured_topology = data::topology_slot<topology_type>;
structured_topology structured;

data::coloring_slot<topology_type> coloring;

#if 0
const field<double>::definition<topology_type, policy::cells> cell_field;
auto pressure = cell_field(structured);
#endif

int
check() {
  UNIT { flog(info) << "check" << std::endl; };
} // check

int
structured_driver() {
  UNIT {
    coloring.allocate();
    // structured.allocate(coloring.get());

    EXPECT_EQ(test<check>(), 0);
  };
} // structured_driver

flecsi::unit::driver<structured_driver> driver;
