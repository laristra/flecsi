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
#include "flecsi/runtime/backend.hh"
#include <flecsi/topology/ntree/interface.hh>
#include <flecsi/topology/set/interface.hh>
#include <flecsi/topology/structured_mesh/interface.hh>
//#include <flecsi/topology/unstructured_mesh/interface.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/tuple_walker.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(task_prologue);

namespace flecsi {
namespace execution {
namespace legion {

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

  task_prologue_t(Legion::Runtime * runtime,
    Legion::Context & context,
    const size_t & domain)
    : runtime_(runtime), context_(context), domain_(domain) {}

  std::vector<Legion::RegionRequirement> const & region_requirements() {
    return region_reqs_;
  } // region_requirements

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
    The following methods are specializations on storage class and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::dense,
               topology::global_t,
               DATA_TYPE,
               PRIVILEGES> * /* parameter */,
    const data::field_reference<DATA_TYPE, topology::global_t> & ref) {
    Legion::LogicalRegion region = ref.topology().get().logical_region;

    static_assert(privilege_count<PRIVILEGES>() == 1,
      "global topology accessor type only takes one privilege");

    constexpr auto priv = get_privilege<0, PRIVILEGES>();

    if(priv > partition_privilege_t::ro)
      flog_assert(domain_ == 1,
        "global can only be modified from within single launch task");

    Legion::RegionRequirement rr(region,
      priv > partition_privilege_t::ro ? privilege_mode(priv) : READ_ONLY,
      EXCLUSIVE,
      region);

    rr.add_field(ref.identifier());
    region_reqs_.push_back(rr);
  } // visit

  /*--------------------------------------------------------------------------*
    Index Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::dense,
               topology::index_t,
               DATA_TYPE,
               PRIVILEGES> * /* parameter */,
    const data::field_reference<DATA_TYPE, topology::index_t> & ref) {
    auto & instance_data = ref.topology().get();

    flog_assert(instance_data.colors == domain_,
      "attempting to pass index topology reference with size "
        << instance_data.colors << " into task with launch domain of size "
        << domain_);

    static_assert(privilege_count<PRIVILEGES>() == 1,
      "index topology accessor type only takes one privilege");

    Legion::RegionRequirement rr(instance_data.color_partition,
      0,
      privilege_mode(get_privilege<0, PRIVILEGES>()),
      EXCLUSIVE,
      instance_data.logical_region);

    rr.add_field(ref.identifier());
    region_reqs_.push_back(rr);
  } // visit

  /*--------------------------------------------------------------------------*
    NTree Topology
   *--------------------------------------------------------------------------*/

  template<typename POLICY_TYPE, size_t PRIVILEGES>
  using ntree_accessor =
    data::topology_accessor<topology::ntree<POLICY_TYPE>, PRIVILEGES>;

  template<class T, typename POLICY_TYPE, size_t PRIVILEGES>
  void visit(ntree_accessor<POLICY_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<T, topology::ntree<POLICY_TYPE>> & ref) {
  } // visit

  /*--------------------------------------------------------------------------*
    Set Topology
   *--------------------------------------------------------------------------*/

  template<typename POLICY_TYPE, size_t PRIVILEGES>
  using set_accessor =
    data::topology_accessor<topology::set<POLICY_TYPE>, PRIVILEGES>;

  template<class T, typename POLICY_TYPE, size_t PRIVILEGES>
  void visit(set_accessor<POLICY_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<T, topology::set<POLICY_TYPE>> & ref) {
  } // visit

  /*--------------------------------------------------------------------------*
    Structured Mesh Topology
   *--------------------------------------------------------------------------*/

  template<typename POLICY_TYPE, size_t PRIVILEGES>
  using structured_mesh_accessor =
    data::topology_accessor<topology::structured_mesh<POLICY_TYPE>, PRIVILEGES>;

  template<class T, typename POLICY_TYPE, size_t PRIVILEGES>
  void visit(
    structured_mesh_accessor<POLICY_TYPE, PRIVILEGES> * /* parameter */,
    const data::field_reference<T, topology::structured_mesh<POLICY_TYPE>> &
      ref) {} // visit

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<class P, typename DATA_TYPE>
  static typename std::enable_if_t<
    !std::is_base_of_v<data::reference_base, DATA_TYPE>>
  visit(P *, DATA_TYPE &) {
    {
      flog_tag_guard(task_prologue);
      flog_devel(info) << "Skipping argument with type "
                       << flecsi::utils::type<DATA_TYPE>() << std::endl;
    }
  } // visit

private:
  template<class... PP, class... AA>
  void walk(std::tuple<PP...> * /* to deduce PP */, const AA &... aa) {
    (visit(static_cast<std::decay_t<PP> *>(nullptr), aa), ...);
  }

  Legion::Runtime * runtime_;
  Legion::Context & context_;
  size_t domain_;

  std::vector<Legion::RegionRequirement> region_reqs_;

}; // task_prologue_t

} // namespace legion
} // namespace execution
} // namespace flecsi
