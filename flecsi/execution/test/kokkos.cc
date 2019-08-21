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

#include <flecsi/utils/ftest.hh>

#include <Kokkos_Core.hpp>

flog_register_tag(kokkos);

/*

 */

int
kokkos_sanity(int argc, char ** argv) {

  FTEST();

  // Kokkos::initialize(argc, argv);
  Kokkos::print_configuration(std::cerr);
  // Kokkos::finalize();

  return FTEST_RESULT();
}

ftest_register_driver(kokkos_sanity);