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

using namespace flecsi;

int
simple2d_16x16() {
  FTEST {
    topo::unstructured_impl::simple_definition sd("simple2d-16x16.msh");

    ASSERT_EQ(sd.dimension(), 2lu);
    ASSERT_EQ(sd.num_entities(0), 289lu);
    ASSERT_EQ(sd.num_entities(2), 256lu);
  };
}

int
driver(int, char **) {
  FTEST {
    // TODO: use test<> when reduction works for MPI tasks
    execute<simple2d_16x16, flecsi::index, mpi>();
  };
}

ftest_register_driver(driver);
