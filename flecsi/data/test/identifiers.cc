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
identifiers(int, char **) {
  FTEST {
    flog(info) << "global: " << topology::id<topology::global>() << std::endl;
    flog(info) << "index: " << topology::id<topology::index>() << std::endl;
  };
}

ftest_register_driver(identifiers);
