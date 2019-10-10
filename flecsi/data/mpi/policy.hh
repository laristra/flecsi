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
#pragma once

#include <flecsi/data/reference.hh>

namespace flecsi {
namespace data {

template<class T>
struct topology_traits {
  static void allocate(reference_base const & topology_reference,
    const typename T::coloring & coloring) {} // allocate

  static void deallocate(reference_base const & topology_reference) {
  } // deallocate
};

} // namespace data
} // namespace flecsi
