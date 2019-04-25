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

#include <cstddef>

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

template<typename ... PRIVILEGES>
struct privilege_pack_u {
  using tuple_t = std::tuple<PRIVILEGES ...>;
  static constexpr size_t value = utils::shift_or<0, tuple_t, 2>();
}; // struct privilege_pack_u

template<size_t INDEX, size_t PACK>
constexpr privilege_t get_privilege() {
  return PACK >> INDEX*2 & 0x03; 
} // get_privilege

#if 0
template<size_t EXCLUSIVE, size_t SHARED, size_t GHOST>
using unstructured_mesh_privileges_t =
  privilege_pack_u<EXCLUSIVE, SHARED, GHOST>;
#endif

#if 0
template<size_t PRIVILEGES>
struct perm {
  template<typename TYPE>
  decltype(auto)
  to() {
    if constexpr(std::is_same_v<TYPE, global>) {
      return PRIVILEGES & 0x03;
    }
    else if(std::is_same_v<TYPE, mesh>) {
      return std::make_tuple(PRIVILEGES >> 4 & 0x03,
        PRIVILEGES >> 2 & 0x03, PRIVILEGES & 0x03);
    }
  }
}

auto to<PRIVILEGES>.global();
#endif

// FIXME: This probably needs to move

template<size_t PRIVILEGES>
constexpr privilege_t
to_global() {
  return static_cast<privilege_t>(PRIVILEGES & 0x03);
} // to_global

} // namespace flecsi
