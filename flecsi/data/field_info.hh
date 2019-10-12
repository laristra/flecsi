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
#else
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/flog.hh>
#include <flecsi/utils/hash.hh>
#endif

#include <cstddef>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace flecsi {
namespace data {

/*!
  The field_info_t type provides a structure for capturing runtime field
  information.
 */

struct field_info_t {
  field_id_t fid = FIELD_ID_MAX;
  size_t index_space = std::numeric_limits<size_t>::max();
  size_t type_size = std::numeric_limits<size_t>::max();
}; // struct field_info_t

using field_info_store_t = std::vector<field_info_t>;

} // namespace data
} // namespace flecsi
