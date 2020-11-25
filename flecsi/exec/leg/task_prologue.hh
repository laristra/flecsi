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
#include "flecsi/exec/leg/future.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/topo/global.hh"
#include "flecsi/topo/set/interface.hh"
#include "flecsi/topo/structured/interface.hh"
//#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/tuple_walker.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <memory>

namespace flecsi {

inline log::devel_tag task_prologue_tag("task_prologue");

namespace exec::leg {

/*!
  The task_prologue_t type can be called to walk task args before the
  task launcher is created. This allows us to gather region requirements
  and to set state on the associated data handles \em before Legion gets
  the task arguments tuple.

  @ingroup execution
*/

struct task_prologue_t {

  std::vector<Legion::RegionRequirement> const & region_requirements() const {
    return region_reqs_;
  } // region_requirements

  std::vector<Legion::Future> && futures() && {
    return std::move(futures_);
  } // futures

  std::vector<Legion::FutureMap> const & future_maps() const {
    return future_maps_;
  } // future_maps

  /*!
    Convert the template privileges to proper Legion privileges.

    @param mode privilege
   */

  static Legion::PrivilegeMode privilege_mode(size_t mode) {
    // Reduce the read and write permissions for each privilege separately:
    bool r = false, w = false;
    for(auto i = privilege_count(mode); i-- && !(r && w);) {
      const auto p = get_privilege(i, mode);
      r = r || privilege_read(p);
      w = w || privilege_write(p);
    }
    return r ? w ? READ_WRITE : READ_ONLY : w ? WRITE_DISCARD : NO_ACCESS;
  } // privilege_mode

  template<class P, class... AA>
  void walk(AA &... aa) {
    walk(static_cast<P *>(nullptr), aa...);
  }

  // All fields are handled in terms of their underlying raw-layout fields.

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on layout and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  template<class T, std::size_t Priv, class Topo, typename Topo::index_space S>
  void visit(data::accessor<data::dense, T, Priv> * null_p,
    const data::field_reference<T, data::dense, Topo, S> & ref) {
    visit(get_null_base(null_p), ref.template cast<data::raw>());
    realloc<S, T, Priv>(ref, [ref] { execute<destroy<T, data::dense>>(ref); });
  }
  template<class T,
    std::size_t Priv,
    class Topo,
    typename Topo::index_space Space>
  void visit(data::accessor<data::single, T, Priv> * null_p,
    const data::field_reference<T, data::single, Topo, Space> & ref) {
    visit(get_null_base(null_p), ref.template cast<data::dense>());
  }

  // This implementation can be generic because all topologies are expected to
  // provide get_region (and, with one exception, get_partition).
  template<typename DATA_TYPE,
    size_t PRIVILEGES,
    auto Space,
    template<class>
    class C,
    class Topo>
  void raw(field_id_t f, C<Topo> & t, bool init = false) {
    data::region & reg = t.template get_region<Space>();

    constexpr auto np = privilege_count(PRIVILEGES);
    static_assert(np == Topo::template privilege_count<Space>,
      "privilege-count mismatch between accessor and topology type");
    if constexpr(np > 1) {
      if(reg.ghost<PRIVILEGES>(f))
        t.template ghost_copy<Space>(f);
    }

    // For convenience, we always use rw accessors for certain fields that
    // initially contain no constructed objects; we must indicate that
    // (vacuous) initialization to Legion.
    const Legion::PrivilegeMode m =
      init ? WRITE_DISCARD : privilege_mode(PRIVILEGES);
    const Legion::LogicalRegion lr = reg.logical_region;
    if constexpr(std::is_same_v<typename Topo::base, topo::global_base>)
      region_reqs_.emplace_back(lr, m, EXCLUSIVE, lr);
    else
      region_reqs_.emplace_back(
        t.template get_partition<Space>(f).logical_partition,
        0,
        m,
        EXCLUSIVE,
        lr);
    region_reqs_.back().add_field(f);
  } // visit

  template<class T, size_t P, class Topo, typename Topo::index_space S>
  void visit(data::accessor<data::raw, T, P> * /* parameter */,
    const data::field_reference<T, data::raw, Topo, S> & ref) {
    raw<T, P, S>(ref.fid(), ref.topology());
  }

  template<class T,
    std::size_t P,
    std::size_t OP,
    class Topo,
    typename Topo::index_space S>
  void ragged(data::ragged_accessor<T, P, OP> * null_p,
    const data::field_reference<T, data::ragged, Topo, S> & f) {
    const field_id_t i = f.fid();
    auto & t = f.topology();
    // It's legitimate, if vacuous, to have the first use of a ragged field be
    // via an accessor (even a read-only one), since all the rows will have a
    // (well-defined) length of 0.
    raw<T, P, S>(i,
      t.ragged,
      t.template get_region<S>().cleanup(
        i,
        [=] {
          if constexpr(!std::is_trivially_destructible_v<T>)
            execute<destroy<T, data::ragged>>(f);
        },
        privilege_write_only(P)));
    visit(
      get_null_offsets(null_p), f.template cast<data::dense, std::size_t>());
  }
  template<class T, std::size_t P, class Topo, typename Topo::index_space S>
  void visit(data::accessor<data::ragged, T, P> * null_p,
    const data::field_reference<T, data::ragged, Topo, S> & f) {
    ragged(null_p, f);
  }
  template<class T, class Topo, typename Topo::index_space S>
  void visit(data::mutator<data::ragged, T> *,
    const data::field_reference<T, data::ragged, Topo, S> & f) {
    auto & p = f.topology().ragged.template get_partition<S>(f.fid());
    p.resize();
    // A mutator doesn't have privileges, so supply the correct number to it:
    ragged(data::mutator<data::ragged,
             T>::template null_base<Topo::template privilege_count<S>>,
      f);
    visit(static_cast<topo::resize::Field::accessor<rw> *>(nullptr), p.sizes());
  }
  template<class T, std::size_t P, class Topo, typename Topo::index_space S>
  void visit(data::accessor<data::sparse, T, P> * null_p,
    const data::field_reference<T, data::sparse, Topo, S> & ref) {
    visit(get_null_base(null_p),
      ref.template cast<data::ragged,
        typename field<T, data::sparse>::base_type::value_type>());
  }
  template<class T, class Topo, typename Topo::index_space S>
  void visit(data::mutator<data::sparse, T> * null_p,
    const data::field_reference<T, data::sparse, Topo, S> & f) {
    visit(get_null_base(null_p),
      f.template cast<data::ragged,
        typename field<T, data::sparse>::base_type::value_type>());
  }

  // Topology accessors use the Visitor pattern: topologies provide a fields
  // function that maps recursive calls to visit over all their fields.
  template<class Topo, std::size_t Priv>
  void visit(data::topology_accessor<Topo, Priv> * /* parameter */,
    data::topology_slot<Topo> & slot) {
    // Clang 10.0.1 deems 'this' unused:
    slot->fields([&](auto & f, auto & s) {
      visit(
        static_cast<data::field_accessor<decltype(f), Priv> *>(nullptr), f(s));
    });
  }

  /*--------------------------------------------------------------------------*
    Futures
   *--------------------------------------------------------------------------*/
  template<class P, class T>
  void visit(P *, const future<T> & f) {
    futures_.push_back(f.legion_future_);
  }

  template<class P, class T>
  void visit(P *, const future<T, exec::launch_type_t::index> & f) {
    future_maps_.push_back(f.legion_future_);
  }

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<class P, class A>
  static std::enable_if_t<!std::is_base_of_v<data::convert_tag, A>> visit(P *,
    const A &) {
    log::devel_guard guard(task_prologue_tag);
    flog_devel(info) << "Skipping argument with type " << util::type<A>()
                     << std::endl;
  } // visit

private:
  template<class T, data::layout L>
  static void destroy(typename field<T, L>::template accessor<rw> a) {
    const auto s = a.span();
    std::destroy(s.begin(), s.end());
  }
  template<auto S, class T, std::size_t Priv, class R, class F>
  static void realloc(const R & ref, F f) {
    // TODO: use just one task for all fields
    if constexpr(privilege_write_only(Priv) &&
                 !std::is_trivially_destructible_v<T>)
      ref.topology().template get_region<S>().cleanup(ref.fid(), std::move(f));
    else
      (void)ref, (void)f;
  }

  template<class... PP, class... AA>
  void walk(std::tuple<PP...> * /* to deduce PP */, AA &... aa) {
    (visit(static_cast<std::decay_t<PP> *>(nullptr), aa), ...);
  }

  std::vector<Legion::RegionRequirement> region_reqs_;
  std::vector<Legion::Future> futures_;
  std::vector<Legion::FutureMap> future_maps_;
}; // task_prologue_t

} // namespace exec::leg
} // namespace flecsi
