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

/*! file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <cstddef> // size_t

#include "flecsi/data/privilege.hh"
#include "flecsi/data/topology_slot.hh"
#include "flecsi/util/constant.hh"
#include "flecsi/util/type_traits.hh"

namespace flecsi {
namespace data {
template<class>
struct coloring_slot; // avoid dependency on flecsi::execute
template<class, std::size_t>
struct topology_accessor; // avoid circularity via launch.hh
} // namespace data

namespace topo {
enum single_space { elements };

namespace detail {
template<template<class> class>
struct base;

inline std::size_t next_id;
} // namespace detail

template<template<class> class T>
using base_t = typename detail::base<T>::type;

template<class T>
using identity = T; // can be a trivial specialization interface

/// CRTP base for specializations.
/// \tparam C core topology
/// \tparam D derived topology type
/// \tparam I specialization interface accepting a base class
template<template<class> class C, class D, template<class> class I = identity>
struct specialization {
  using core = C<D>;
  using base = base_t<C>;
  // This is just core::coloring, but core is incomplete here.
  using coloring = typename base::coloring;
  template<class B>
  using interface = I<B>;

  // NB: nested classes would prevent template argument deduction.
  using slot = data::topology_slot<D>;
  using cslot = data::coloring_slot<D>;

  /// The topology accessor to use as a parameter to receive a \c slot.
  /// \tparam Priv the appropriate number of privileges
  template<partition_privilege_t... Priv>
  using accessor = data::topology_accessor<D, privilege_pack<Priv...>>;

  // Use functions because these are needed during non-local initialization:
  static std::size_t id() {
    static auto ret = detail::next_id++;
    return ret;
  }

  // May be overridden by policy:
  using index_space = single_space;
  using index_spaces = util::constants<elements>;
  // Most compilers eagerly instantiate a deduced static member type, so we
  // have to use a function.
  static constexpr auto default_space() {
    return D::index_spaces::value;
  }
  template<auto S> // we can't use D::index_space here
  static constexpr std::size_t privilege_count =
    std::is_same_v<decltype(S), typename D::index_space> ? 1 : throw;

  specialization() = delete;
};

} // namespace topo
} // namespace flecsi
