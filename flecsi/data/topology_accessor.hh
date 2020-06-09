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

#include "flecsi/exec/launch.hh"
#include <cstddef> // size_t

namespace flecsi {
namespace data {

/*!
  Topology accessor type. Topology accessors are defined by the interface of
  the underlying, user-defined type, i.e., unlike field accessors,
  topologies can be customized by the specialization to add types and inerfaces
  that are not part of the core FleCSI topology type. By inheriting from the
  customized topology type, we pick up these additions.

  \tparam T topology type
  \tparam Priv privilege pack
 */
template<class T, std::size_t Priv>
struct topology_accessor
  : T::template interface<typename T::core::template access<Priv>> {
  using core = typename T::core::template access<Priv>;
  static_assert(sizeof(typename T::template interface<core>) == sizeof(core),
    "topology interfaces may not add data members");

  explicit topology_accessor() = default;
}; // struct topology_accessor

} // namespace data

template<class T, std::size_t P>
struct exec::detail::task_param<data::topology_accessor<T, P>> {
  static auto replace(const typename T::slot &) {
    return data::topology_accessor<T, P>();
  }
};
} // namespace flecsi
