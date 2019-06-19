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
#include <flecsi/data/data.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

int
identifiers(int argc, char ** argv) {

  FTEST();

  flog(info) << "global_topology_t: "
             << topology::global_topology_t::type_identifier_hash << std::endl;
  flog(info) << "index_topology_t: "
             << topology::index_topology_t::type_identifier_hash << std::endl;

  auto ih = flecsi_topology_reference(
    flecsi::topology::global_topology_t, "internal", "global_topology");
  flog(info) << "index topology handle " << ih.identifier() << std::endl;

  return FTEST_RESULT();
}

ftest_register_driver(identifiers);
