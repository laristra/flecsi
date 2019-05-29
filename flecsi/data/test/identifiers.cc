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
#include <flecsi/data/data.h>
#include <flecsi/topology/internal/global.h>
#include <flecsi/topology/internal/index.h>
#include <flecsi/utils/ftest.h>

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
}

ftest_register_driver(identifiers);
