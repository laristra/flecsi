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

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/data/field.hh"
#include "flecsi/data/privilege.hh"
#include "flecsi/exec/launch.hh"
#include "flecsi/run/context.hh"
#include "flecsi/util/demangle.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

inline log::devel_tag unbind_accessors_tag("unbind_accessors");

namespace exec::leg {

// Note that what is visited are the objects \e moved into the user's
// parameters (and are thus the same object only in case of a reference).
struct unbind_base {
  template<data::layout L, typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<L, DATA_TYPE, PRIVILEGES> &) {} // visit

  template<data::layout L, class T, std::size_t P>
  void visit(data::mutator<L, T, P> & m) {
    m.commit();
  }

  template<class Topo, std::size_t P>
  void visit(data::topology_accessor<Topo, P> &) {}

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static
    typename std::enable_if_t<!std::is_base_of_v<data::bind_tag, DATA_TYPE>>
    visit(DATA_TYPE &) {
    {
      log::devel_guard guard(unbind_accessors_tag);
      flog_devel(info) << "No cleanup for parameter of type "
                       << util::type<DATA_TYPE>() << std::endl;
    }
  } // visit
};

template<class... TT>
struct unbind_accessors : unbind_base {
  using Tuple = std::tuple<TT...>;

  unbind_accessors(Tuple & t) : acc(t) {
    buffer(std::index_sequence_for<TT...>());
  }

  void operator()() {
    std::apply(
      [this](TT &... tt) {
        (void)this; // to appease Clang
        (visit(tt), ...);
      },
      acc);
  }

private:
  template<std::size_t... II>
  void buffer(std::index_sequence<II...>) {
    (set_buffer(std::get<II>(acc), std::get<II>(buf)), ...);
  }

  Tuple & acc;
  std::tuple<buffer_t<TT>...> buf;
};

} // namespace exec::leg
} // namespace flecsi
