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

  flog(info) << "global_t: " << topology::id<topology::global_t>() << std::endl;
  flog(info) << "index_t: " << topology::id<topology::index_t>() << std::endl;

  return FTEST_RESULT();
}

ftest_register_driver(identifiers);
