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

struct block : topo::specialization<topo::structured, block> {
  static coloring color() {
    coloring c = 10;
    return c;
  } // color
};

block::slot structured;
block::cslot coloring;

#if 0
const field<double>::definition<block, block::cells> cell_field;
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
