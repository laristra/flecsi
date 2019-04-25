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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/utils/shift_or.h>
#endif

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

enum privilege_t : size_t {
  nu = 0,
  ro = 1,
  wo = 2,
  rw = 3
}; // enum privilege_t

/*!
  Utility type to allow general privilege components that will match the old
  style of specifying permissions, e.g., <EX, SH, GH> (The old approach was
  only valid for mesh type topologies, and didn't make sense for all topology
  types).

  @tparam PRIVILEGES A variadic list of typeified privilege_t elements.
 */

template<typename ... PRIVILEGES>
struct privilege_pack_u {
  using tuple_t = std::tuple<PRIVILEGES ...>;
  static constexpr size_t value = utils::shift_or<0, tuple_t, 2>();
}; // struct privilege_pack_u

/*!
  Get a privilege out of a pack for the specified id.

  @tparam INDEX The index of the privilege to get.
  @tparam PACK  A valid privilege_pack_u type.
  @tparam COUNT The number of privileges stored in the pack.
 */

template<size_t INDEX, size_t PACK, size_t COUNT>
constexpr privilege_t get_privilege() {
  return privilege_t(PACK >> (COUNT - 1 - INDEX)*2 & 0x03);
} // get_privilege

template<size_t PRIVILEGES>
constexpr privilege_t
to_global() {
  return static_cast<privilege_t>(PRIVILEGES & 0x03);
} // to_global

} // namespace flecsi
