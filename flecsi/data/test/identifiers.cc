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
#include <flecsi/data.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

int
identifiers(int argc, char ** argv) {

  FTEST();

  flog(info) << "global_topology_t: "
             << topology::id<topology::global_topology_t>() << std::endl;
  flog(info) << "index_topology_t: "
             << topology::id<topology::index_topology_t>() << std::endl;

  auto ih = flecsi_topology_reference(
    flecsi::topology::global_topology_t, "internal", "global_topology");

  flog(info) << "index topology handle " << ih.identifier() << std::endl;

  return FTEST_RESULT();
}

ftest_register_driver(identifiers);
