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
#include "flecsi/data/topology_accessor.hh"
#include "flecsi/data/topology_slot.hh"
#include "flecsi/util/type_traits.hh"

namespace flecsi {
namespace data {
template<class>
struct coloring_slot; // avoid dependency on flecsi::execute
}

namespace topo {
enum single_space { elements };

namespace detail {
template<template<class> class>
struct base;

inline std::size_t next_id;
// Use functions because these are needed during non-local initialization:
template<class>
std::size_t
id() {
  static auto ret = next_id++;
  return ret;
}

template<class, class = void>
struct index_space {
  using type = single_space;
  static constexpr single_space default_space = elements;
};
template<class T> // TIP: SFINAE uses index_space member if defined
struct index_space<T, util::voided<typename T::index_space>> {
  using type = typename T::index_space;
};

// With this as a variable template, GCC 8.1 incorrectly produces a hard error
// rather than discounting the partial specialization when appropriate.
template<class T, class = void>
struct default_space { // NB: not SFINAE-friendly
  static constexpr single_space value = detail::index_space<T>::default_space;
};
template<class T>
struct default_space<T, decltype(void(T::default_space))> {
  static constexpr auto value = T::default_space;
};
} // namespace detail

template<template<class> class T>
using base_t = typename detail::base<T>::type;

template<class T>
std::size_t
id() {
  return detail::id<std::remove_cv_t<T>>();
}

template<class, class = void>
inline constexpr std::size_t index_spaces = 1;
// TIP: expression SFINAE uses index_spaces member if defined
template<class T>
inline constexpr std::size_t
  index_spaces<T, decltype(void(T::core::index_spaces))> =
    T::core::index_spaces;

template<class T>
using index_space_t = typename detail::index_space<typename T::core>::type;
template<class T>
inline constexpr auto default_space =
  detail::default_space<typename T::core>::value;
template<class T, index_space_t<T> S, class = const std::size_t>
inline constexpr std::size_t privilege_count = 1;
template<class T, index_space_t<T> S>
inline constexpr std::size_t
  privilege_count<T, S, decltype(T::core::template privilege_count<S>)> =
    T::core::template privilege_count<S>;

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

  specialization() = delete;
};

} // namespace topo
} // namespace flecsi
