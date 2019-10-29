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

#include "flecsi/data/privilege.hh"
#include "flecsi/data/storage_classes.hh"
#include "flecsi/runtime/backend.hh"
#include "flecsi/topology/core.hh"
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/tuple_walker.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(bind_accessors);

namespace flecsi {
namespace execution {
namespace legion {

/*!
  The bind_accessors_t type is called to walk the user task arguments inside of
  an executing legion task to properly complete the users accessors, i.e., by
  pointing the accessor \em view instances to the appropriate legion-mapped
  buffers.
 */

struct bind_accessors_t : public flecsi::utils::tuple_walker<bind_accessors_t> {

  /*!
    Construct an bind_accessors_t instance.

    @param legion_runtime The Legion task runtime.
    @param legion_context The Legion task runtime context.
   */

  bind_accessors_t(Legion::Runtime * legion_runtime,
    Legion::Context & legion_context,
    std::vector<Legion::PhysicalRegion> const & regions,
    std::vector<Legion::Future> const & futures)
    : legion_runtime_(legion_runtime), legion_context_(legion_context),
      regions_(regions), futures_(futures) {}

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on storage class and topology
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  template<class A>
  void dense(A & accessor) {
    using DATA_TYPE = typename A::value_type;

    auto & reg = regions_[region++];

    //    Legion::FieldAccessor<privilege_mode(get_privilege<0, PRIVILEGES>()),
    const Legion::UnsafeFieldAccessor<DATA_TYPE,
      1,
      Legion::coord_t,
      Realm::AffineAccessor<DATA_TYPE, 1, Legion::coord_t>>
      ac(reg, accessor.identifier(), sizeof(DATA_TYPE));

    bind(accessor,
      ac.ptr(Legion::Domain::DomainPointIterator(
        legion_runtime_->get_index_space_domain(
          legion_context_, reg.get_logical_region().get_index_space()))
               .p));
  }

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::accessor<data::dense,
    topology::global_topology_t,
    DATA_TYPE,
    PRIVILEGES> & accessor) {
    dense(accessor);
  }

  /*--------------------------------------------------------------------------*
    Index Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(data::
      accessor<data::dense, topology::index_topology_t, DATA_TYPE, PRIVILEGES> &
        accessor) {
    dense(accessor);
  }

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static typename std::enable_if_t<
    !std::is_base_of_v<data::reference_base, DATA_TYPE>>
  visit(DATA_TYPE &) {
    {
      flog_tag_guard(bind_accessors);
      flog_devel(info) << "Skipping argument with type "
                       << flecsi::utils::type<DATA_TYPE>() << std::endl;
    }
  } // visit

private:
  Legion::Runtime * legion_runtime_;
  Legion::Context & legion_context_;
  size_t region = 0;
  const std::vector<Legion::PhysicalRegion> & regions_;
  size_t future = 0;
  const std::vector<Legion::Future> & futures_;

}; // struct bind_accessors_t

} // namespace legion
} // namespace execution
} // namespace flecsi
