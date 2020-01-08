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
  FTEST();

  topology::unstructured_impl::simple_definition sd("simple2d-16x16.msh");

  ASSERT_EQ(sd.dimension(), 2lu);
  ASSERT_EQ(sd.num_entities(0), 289lu);
  ASSERT_EQ(sd.num_entities(1), 256lu);

  return FTEST_RESULT();
}

int
driver(int, char **) {

  execute<simple2d_16x16, flecsi::index, mpi>();

  return 0;
}

ftest_register_driver(driver);
