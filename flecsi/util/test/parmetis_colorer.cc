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

#include "flecsi/util/graph/parmetis_colorer.hh"
#include "flecsi/topo/unstructured/simple_definition.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/execution.hh>

using namespace flecsi;

int
simple2d_16x16() {
  UNIT { topo::simple_definition sd("simple2d-16x16.msh"); };
} // simple2d_16x16

int
colorer() {
  UNIT { EXPECT_EQ((test<simple2d_16x16, mpi>()), 0); };
} // colorer

flecsi::unit::driver<colorer> driver;
