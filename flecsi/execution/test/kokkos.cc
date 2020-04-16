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

log::devel_tag kokkos_tag("kokkos");

/*

 */

int
kokkos_sanity(int, char **) {
  FTEST {
    // Kokkos::initialize(argc, argv);
    Kokkos::print_configuration(std::cerr);
    // Kokkos::finalize();
  };
}

ftest_register_driver(kokkos_sanity);
