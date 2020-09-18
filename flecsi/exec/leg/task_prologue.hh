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
    switch(mode) {
      case size_t(nu):
        return WRITE_DISCARD;
      case size_t(ro):
        return READ_ONLY;
      case size_t(wo):
        return WRITE_DISCARD;
      case size_t(rw):
        return READ_WRITE;
      default:
        flog_fatal("invalid privilege mode");
    } // switch

    return NO_ACCESS;
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
  void visit(data::accessor<data::singular, T, Priv> * null_p,
    const data::field_reference<T, data::singular, Topo, Space> & ref) {
    visit(get_null_base(null_p), ref.template cast<data::dense>());
  }

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::raw, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::
      field_reference<DATA_TYPE, data::raw, topo::global, topo::elements> &
        ref) {
    Legion::LogicalRegion region = ref.topology().logical_region;

    static_assert(privilege_count(PRIVILEGES) == 1,
      "global topology accessor type only takes one privilege");

    constexpr auto priv = get_privilege(0, PRIVILEGES);

    Legion::RegionRequirement rr(region,
      priv > partition_privilege_t::ro ? privilege_mode(priv) : READ_ONLY,
      EXCLUSIVE,
      region);

    rr.add_field(ref.fid());
    region_reqs_.push_back(rr);
  } // visit

  // This implementation can be generic because all topologies are expected to
  // provide get_partition.
  template<typename DATA_TYPE,
    size_t PRIVILEGES,
    class Topo,
    typename Topo::index_space Space,
    class = std::enable_if_t<Topo::template privilege_count<Space> == 1>>
  void visit(data::accessor<data::raw, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<DATA_TYPE, data::raw, Topo, Space> & ref) {
    auto & instance_data =
      ref.topology().template get_partition<Space>(ref.fid());

    static_assert(privilege_count(PRIVILEGES) == 1,
      "accessors for this topology type take only one privilege");

    Legion::RegionRequirement rr(instance_data.logical_partition,
      0,
      privilege_mode(get_privilege(0, PRIVILEGES)),
      EXCLUSIVE,
      Legion::Runtime::get_runtime()->get_parent_logical_region(
        instance_data.logical_partition));

    rr.add_field(ref.fid());
    region_reqs_.push_back(rr);
  } // visit

  template<class T,
    std::size_t P,
    std::size_t OP,
    class Topo,
    typename Topo::index_space S>
  void ragged(data::ragged_accessor<T, P, OP> * null_p,
    const data::field_reference<T, data::ragged, Topo, S> & f) {
    // We rely on the fact that field_reference uses only the field ID.
    visit(get_null_base(null_p),
      data::field_reference<T, data::raw, topo::ragged_topology<Topo>, S>(
        {f.fid(), 0}, f.topology().ragged));
    visit(
      get_null_offsets(null_p), f.template cast<data::dense, std::size_t>());
    realloc<S, T, P>(f, [=] { execute<destroy<T, data::ragged>>(f); });
  }
  template<class T, std::size_t P, class Topo, typename Topo::index_space S>
  void visit(data::accessor<data::ragged, T, P> * null_p,
    const data::field_reference<T, data::ragged, Topo, S> & f) {
    ragged(null_p, f);
  }
  template<class T, class Topo, typename Topo::index_space S>
  void visit(data::mutator<data::ragged, T> *,
    const data::field_reference<T, data::ragged, Topo, S> & f) {
    // A mutator doesn't have privileges, so supply the correct number to it:
    ragged(data::mutator<data::ragged,
             T>::template null_base<Topo::template privilege_count<S>>,
      f);
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
  template<typename DATA_TYPE>
  void visit(const future<DATA_TYPE> & f) {
    futures_.push_back(f.legion_future_);
  }

  template<typename DATA_TYPE>
  void visit(const future<DATA_TYPE, exec::launch_type_t::index> & f) {
    future_maps_.push_back(f.legion_future_);
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

  // Argument types for which we don't also need the type of the parameter:
  template<class P, typename DATA_TYPE>
  void visit(P *, const DATA_TYPE & x) {
    visit(x);
  } // visit

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
