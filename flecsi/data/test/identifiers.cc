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
#include "flecsi/util/ftest.hh"
#include <flecsi/data.hh>

using namespace flecsi;

int
identifiers(int, char **) {
  FTEST {
    flog(info) << "global: " << topo::id<topo::global>() << std::endl;
    flog(info) << "index: " << topo::id<topo::index>() << std::endl;
  };
}

ftest_register_driver(identifiers);
