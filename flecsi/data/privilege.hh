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
#endif

#include "flecsi/util/bitutils.hh"

#include <cstddef>
#include <tuple>
#include <utility>

namespace flecsi {

/*!
  Enumeration for specifying access privleges for data that are passed
  to FleCSI tasks.

  @param nu No-Update access: data are mapped, no updates are done to
            the state. This privilege is basically \em as \em is.
  @param ro Read-Only access: data are mapped, updates are performed for
            consistency, but the data are read-only.
  @param wo Write-Only access: data are mapped, no updates are done to the
            state, and the data can be written.
  @param rw Read-Write access: data are mapped, updated are performend for
            consistency, and the data are read-write.
 */

enum partition_privilege_t : size_t {
  nu = 0b00,
  ro = 0b01,
  wo = 0b10,
  rw = 0b11
}; // enum partition_privilege_t

inline constexpr short privilege_bits = 2;

/*!
  Utility to allow general privilege components that will match the old
  style of specifying permissions, e.g., <EX, SH, GH> (The old approach was
  only valid for mesh type topologies, and didn't make sense for all topology
  types).

  \tparam PP privileges
 */
template<partition_privilege_t... PP>
inline constexpr size_t privilege_pack = [] {
  static_assert(((PP < 1 << privilege_bits) && ...));
  std::size_t ret = 1; // nonzero to allow recovering sizeof...(PP)
  ((ret <<= privilege_bits, ret |= PP), ...);
  return ret;
}();

/*!
  Return the number of privileges stored in a privilege pack.

  \param PACK a \c privilege_pack value
 */

constexpr size_t
privilege_count(std::size_t PACK) {
  return (util::bit_width(PACK) - 1) / privilege_bits;
} // privilege_count

/*!
  Get a privilege out of a pack for the specified id.

  \param i privilege index
  \param pack a \c privilege_pack value
 */

constexpr partition_privilege_t
get_privilege(std::size_t i, std::size_t pack) {
  return partition_privilege_t(
    pack >> (privilege_count(pack) - 1 - i) * privilege_bits &
    ((1 << privilege_bits) - 1));
} // get_privilege

constexpr bool
privilege_write(std::size_t pack) noexcept {
  for(auto i = privilege_count(pack); i--;)
    if(get_privilege(i, pack) > ro)
      return true;
  return false;
}

constexpr bool
privilege_write_only(std::size_t pack) noexcept {
  for(auto i = privilege_count(pack); i--;)
    if(get_privilege(i, pack) != wo)
      return false;
  return true;
}

} // namespace flecsi
