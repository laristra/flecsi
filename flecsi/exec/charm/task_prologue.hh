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
//#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/tuple_walker.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

inline log::devel_tag task_prologue_tag("task_prologue");

namespace exec::charm {

/*!
  The task_prologue_t type can be called to walk task args before the
  task launcher is created. This allows us to gather region requirements
  and to set state on the associated data handles \em before Legion gets
  the task arguments tuple.

  @ingroup execution
*/

struct task_prologue_t {

  /*!
    Construct an task_prologue_t instance.

    @param runtime The Legion task runtime.
    @param context The Legion task runtime context.
   */

  task_prologue_t(const size_t & domain) : domain_(domain) {}

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
  void walk(const AA &... aa) {
    walk(static_cast<P *>(nullptr), aa...);
  }

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on layout and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  template<class T,
    std::size_t Priv,
    class Topo,
    topo::index_space_t<Topo> Space>
  void visit(data::accessor<data::singular, T, Priv> * null_p,
    const data::field_reference<T, data::singular, Topo, Space> & ref) {
    visit(get_null_base(null_p), ref.template cast<data::dense>());
  }

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(
    data::accessor<data::dense, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::
      field_reference<DATA_TYPE, data::dense, topo::global, topo::elements> &
        ref) {
    Legion::LogicalRegion region = ref.topology().get().logical_region;

    static_assert(privilege_count(PRIVILEGES) == 1,
      "global topology accessor type only takes one privilege");

    constexpr auto priv = get_privilege(0, PRIVILEGES);

    if(priv > partition_privilege_t::ro)
      flog_assert(domain_ == 1,
        "global can only be modified from within single launch task");

    Legion::RegionRequirement rr(region,
      priv > partition_privilege_t::ro ? privilege_mode(priv) : READ_ONLY,
      EXCLUSIVE,
      region);

    rr.add_field(ref.fid());
    region_reqs_.push_back(rr);
  } // visit

  template<typename DATA_TYPE,
    size_t PRIVILEGES,
    class Topo,
    topo::index_space_t<Topo> Space,
    class = std::enable_if_t<topo::privilege_count<Topo, Space> == 1>>
  void visit(
    data::accessor<data::dense, DATA_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<DATA_TYPE, data::dense, Topo, Space> & ref) {
    auto & instance_data = ref.topology().get().template get_partition<Space>();

    flog_assert(instance_data.colors() == domain_,
      "attempting to pass field with "
        << instance_data.colors()
        << " partitions into task with launch domain of size " << domain_);

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

  template<class Topo, std::size_t Priv>
  void visit(data::topology_accessor<Topo, Priv> * /* parameter */,
    const data::topology_slot<Topo> & slot) {
    Topo::core::fields([&](auto & f) {
      visit(static_cast<data::field_accessor<decltype(f), Priv> *>(nullptr),
        f(slot));
    });
  }

  /*--------------------------------------------------------------------------*
    Futures
   *--------------------------------------------------------------------------*/
  template<typename DATA_TYPE>
  void visit(exec::flecsi_future<DATA_TYPE, launch_type_t::single> *,
    const exec::legion_future<DATA_TYPE, exec::launch_type_t::single> &
      future) {
    futures_.push_back(future.legion_future_);
  }

  template<typename DATA_TYPE>
  void visit(exec::flecsi_future<DATA_TYPE, launch_type_t::single> *,
    const exec::legion_future<DATA_TYPE, exec::launch_type_t::index> & future) {
    future_maps_.push_back(future.legion_future_);
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
  // Argument types for which we don't also need the type of the parameter:
  template<class P, typename DATA_TYPE>
  void visit(P *, DATA_TYPE & x) {
    visit(x);
  } // visit

  template<class... PP, class... AA>
  void walk(std::tuple<PP...> * /* to deduce PP */, const AA &... aa) {
    (visit(static_cast<std::decay_t<PP> *>(nullptr), aa), ...);
  }

  size_t domain_;

  std::vector<Legion::RegionRequirement> region_reqs_;
  std::vector<Legion::Future> futures_;
  std::vector<Legion::FutureMap> future_maps_;
}; // task_prologue_t

} // namespace exec::charm
} // namespace flecsi
