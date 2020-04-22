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

#include "flecsi/util/unit.hh"

#include <Kokkos_Core.hpp>

log::devel_tag kokkos_tag("kokkos");

/*

 */

int
kokkos_sanity(int, char **) {
  UNIT {
    // Kokkos::initialize(argc, argv);
    Kokkos::print_configuration(std::cerr);
    // Kokkos::finalize();
  };
}

flecsi::unit::driver<kokkos_sanity> driver;
