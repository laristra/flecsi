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

#include <bitset>
#include <ostream>

#include <flecsi/utils/target.h>

namespace flecsi {
namespace data {

template<typename T>
struct sparse_entry_value_u {
  using index_t = uint64_t;

  FLECSI_INLINE_TARGET
  sparse_entry_value_u(index_t entry) : entry(entry) {}

  FLECSI_INLINE_TARGET
  sparse_entry_value_u(index_t entry, T value) : entry(entry), value(value) {}

  FLECSI_INLINE_TARGET
  sparse_entry_value_u() {}

  FLECSI_INLINE_TARGET
  bool operator<(const sparse_entry_value_u & ev) const {
    return entry < ev.entry;
  }

  index_t entry;
  T value;
};

template<typename T>
std::ostream &
operator<<(std::ostream & ostr,
  const flecsi::data::sparse_entry_value_u<T> & ev) {
  return ostr << "(" << ev.entry << ", " << ev.value << ")";
} // operator<<

// Generic bitfield type
using bitset_t = std::bitset<8>;

} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
