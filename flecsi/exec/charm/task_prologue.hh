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

#include "flecsi/data/accessor.hh"
#include "flecsi/data/privilege.hh"
#include "flecsi/data/topology_accessor.hh"
#include "flecsi/exec/charm/future.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/topo/global.hh"
#include "flecsi/topo/ntree/interface.hh"
#include "flecsi/topo/set/interface.hh"
#include "flecsi/topo/structured/interface.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/tuple_walker.hh"

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Charm!
#endif

namespace flecsi {

inline log::devel_tag task_prologue_tag("task_prologue");

namespace exec::charm {

/*!
  The task_prologue_t type can be called to walk task args before the
  task launcher is created. For now this is used to register data with
  the Charm++ backend.

  @ingroup execution
*/

struct task_prologue_t {

  template<class P, class... AA>
  void walk(AA &... aa) {
    walk(static_cast<P *>(nullptr), aa...);
  }

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on layout and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  template<class T, std::size_t Priv, class Topo, typename Topo::index_space S>
  void visit(data::accessor<data::dense, T, Priv> * null_p,
    const data::field_reference<T, data::dense, Topo, S> & ref) {
    visit(get_null_base(null_p), ref.template cast<data::raw>());
    // TODO: use just one task for all fields
    if constexpr(privilege_write_only(Priv) &&
                 !std::is_trivially_destructible_v<T>)
      ref.topology()->template get_region<S>().cleanup(
        ref.fid(), [ref] { execute<destroy<T>>(ref); });
  }
  template<class T,
    std::size_t Priv,
    class Topo,
    typename Topo::index_space Space>
  void visit(data::accessor<data::singular, T, Priv> * null_p,
    const data::field_reference<T, data::singular, Topo, Space> & ref) {
    visit(get_null_base(null_p), ref.template cast<data::dense>());
  }

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(
    data::accessor<data::raw, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::
      field_reference<DATA_TYPE, data::raw, topo::global, topo::elements> &
        ref) {
    auto & flecsi_context = run::context::instance();
    flecsi_context.regField(ref.fid(), sizeof(DATA_TYPE));
  } // visit

  template<typename DATA_TYPE,
    size_t PRIVILEGES,
    class Topo,
    typename Topo::index_space Space,
    class = std::enable_if_t<Topo::template privilege_count<Space> == 1>>
  void visit(
    data::accessor<data::raw, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<DATA_TYPE, data::raw, Topo, Space> & ref) {
    auto & flecsi_context = run::context::instance();
    flecsi_context.regField(ref.fid(), sizeof(DATA_TYPE));
  } // visit

  template<class Topo, std::size_t Priv>
  void visit(data::topology_accessor<Topo, Priv> * /* parameter */,
    data::topology_slot<Topo> & slot) {
    Topo::core::fields([&](auto & f) {
      visit(static_cast<data::field_accessor<decltype(f), Priv> *>(nullptr),
        f(slot));
    });
  }

  /*--------------------------------------------------------------------------*
    Futures
   *--------------------------------------------------------------------------*/
  template<typename DATA_TYPE>
  void visit(const future<DATA_TYPE, exec::launch_type_t::single> & f) {
    CkAbort("Futures not yet supported\n");
  }

  template<typename DATA_TYPE>
  void visit(const future<DATA_TYPE, exec::launch_type_t::index> & f) {
    CkAbort("Futures not yet supported\n");
  }

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static void visit(DATA_TYPE &) {
    static_assert(!std::is_base_of_v<data::convert_tag, DATA_TYPE>,
      "Unknown task argument type");
    {
      log::devel_guard guard(task_prologue_tag);
      flog_devel(info) << "Skipping argument with type "
                       << util::type<DATA_TYPE>() << std::endl;
    }
  } // visit

private:
  template<class T>
  static void destroy(typename field<T>::template accessor<rw> a) {
    const auto s = a.span();
    std::destroy(s.begin(), s.end());
  }

  // Argument types for which we don't also need the type of the parameter:
  template<class P, typename DATA_TYPE>
  void visit(P *, const DATA_TYPE & x) {
    visit(x);
  } // visit

  template<class... PP, class... AA>
  void walk(std::tuple<PP...> * /* to deduce PP */, AA &... aa) {
    (visit(static_cast<std::decay_t<PP> *>(nullptr), aa), ...);
  }

}; // task_prologue_t

} // namespace exec::charm
} // namespace flecsi
