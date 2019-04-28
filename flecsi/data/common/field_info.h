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

/*!  @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <cstddef>
#include <limits>

namespace flecsi {
namespace data {

/*!

 */

struct field_info_t {
  size_t namespace_hash = std::numeric_limits<size_t>::max();
  size_t name_hash = std::numeric_limits<size_t>::max();
  size_t type_size = std::numeric_limits<size_t>::max();
  size_t versions = std::numeric_limits<size_t>::max();
  size_t fid = std::numeric_limits<size_t>::max();
  size_t index_space = std::numeric_limits<size_t>::max();
  size_t key = std::numeric_limits<size_t>::max();
}; // struct field_info_t

} // namespace data
} // namespace flecsi
