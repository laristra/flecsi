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

#include "flecsi/data/privilege.hh"
#include "flecsi/topo/index.hh"

namespace flecsi {
namespace topo {

struct global_base {
  struct coloring {};
};

template<class P>
struct global_category : global_base, data::region, with_ragged<P> {
  global_category(const coloring &)
    : region(data::make_region<P>({1, 1})), with_ragged<P>(1) {}
};
template<>
struct detail::base<global_category> {
  using type = global_base;
};

/*!
  The \c global type allows users to register data on a
  topology with a single index, i.e., there is one instance of
  the registered field type that is visible to all colors.

  @ingroup topology
 */
struct global : specialization<global_category, global> {};

} // namespace topo

// Defined here to avoid circularity via ragged and execute.
template<data::layout L, class T, std::size_t Priv>
struct exec::detail::launch<data::accessor<L, T, Priv>,
  data::field_reference<T, L, topo::global, topo::elements>> {
  static std::conditional_t<privilege_write(get_privilege(0, Priv)),
    std::monostate,
    std::nullptr_t>
  get(const data::field_reference<T, L, topo::global, topo::elements> &) {
    return {};
  }
};

} // namespace flecsi
