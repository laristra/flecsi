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

#include <flecsi/execution.hh>
#include <flecsi/topology/unstructured/simple_definition.hh>
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/graph/parmetis_colorer.hh>

using namespace flecsi;

int
simple2d_16x16() {
  FTEST { topology::simple_definition sd("simple2d-16x16.msh"); };
} // simple2d_16x16

int
colorer_driver(int argc, char ** argv) {

  execute<simple2d_16x16, flecsi::index, mpi>();

  return 0;
} // simple2d_8x8

ftest_register_driver(colorer_driver);
