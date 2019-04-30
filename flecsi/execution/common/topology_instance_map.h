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

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/utils/flog.h>
#endif

#include <unordered_map>

namespace flecsi {
namespace execution {

template<typename TOPOLOGY_TYPE>
struct topology_instance_map_u {

  //--------------------------------------------------------------------------//
  // Coloring types
  //--------------------------------------------------------------------------//

  using coloring_t = typename TOPOLOGY_TYPE::coloring_t;

  struct coloring_map_t : public std::unordered_map<size_t, coloring_t> {};
  using instance_map_t = std::unordered_map<size_t, coloring_map_t>;

  //--------------------------------------------------------------------------//
  // Launch mapping types
  //--------------------------------------------------------------------------//

private:


}; // struct topology_instance_map_u

} // namespace execution
} // namespace flecsi
