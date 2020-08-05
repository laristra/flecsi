/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

namespace flecsi {
namespace topology {
namespace structured_impl {

/*enum partition_t : size_t {
  overlay = 1,
  exclusive = 2,
  shared = 3,
  ghost = 4,
  domain_halo = 5
}; */

enum class partition_t { overlay, exclusive, shared, ghost, domain_halo };

} // namespace structured_impl
} // namespace topology
} // namespace flecsi
