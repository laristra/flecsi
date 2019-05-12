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
#else
#include <flecsi/data/common/privilege.h>
#include <flecsi/data/legion/storage_classes.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/demangle.h>
#include <flecsi/utils/tuple_walker.h>
#endif

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(init_views);

namespace flecsi {
namespace execution {
namespace legion {

using namespace flecsi::data::legion;

/*!
  The init_views_t type is called to walk the user task arguments inside of
  an executing legion task to properly complete the users accessors, i.e., by
  pointing the accessor \em view instances to the appropriate legion-mapped
  buffers.
 */

struct init_views_t : public flecsi::utils::tuple_walker_u<init_views_t> {

  /*!
    Construct an init_views_t instance.

    @param legion_runtime The Legion task runtime.
    @param legion_context The Legion task runtime context.
   */

  init_views_t(Legion::Runtime * legion_runtime,
    Legion::Context & legion_context,
    std::vector<Legion::PhysicalRegion> const & regions,
    std::vector<Legion::Future> const & futures)
    : legion_runtime_(legion_runtime), legion_context_(legion_context),
      regions_(regions), futures_(futures) {}

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on storage class and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(global_topology::accessor_u<DATA_TYPE, PRIVILEGES> & accessor) {

    Legion::PhysicalRegion pr = regions_[region];
    Legion::LogicalRegion lr = pr.get_logical_region();
    Legion::IndexSpace is = lr.get_index_space();

    const auto fid =
      context_t::instance()
        .get_field_info_store(flecsi_internal_type_hash(global_topology_t),
          data::storage_label_t::global)
        .get_field_info(accessor.identifier())
        .fid;

    auto ac = pr.get_field_accessor(fid).template typeify<DATA_TYPE>();

    Legion::Domain domain =
      legion_runtime_->get_index_space_domain(legion_context_, is);
    LegionRuntime::Arrays::Rect<1> dr = domain.get_rect<1>();
    LegionRuntime::Arrays::Rect<1> sr;
    LegionRuntime::Accessor::ByteOffset bo[2];

    global_topology::data_binder_u<DATA_TYPE, PRIVILEGES>::bind(accessor,
      ac.template raw_rect_ptr<1>(dr, sr, bo));

    ++region;
  } // visit

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static typename std::enable_if_t<
    !std::is_base_of_v<data::data_reference_base_t, DATA_TYPE>>
  visit(DATA_TYPE &) {
    {
      flog_tag_guard(init_views);
      flog(internal) << "Skipping argument with type "
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

}; // struct init_views_t

} // namespace legion
} // namespace execution
} // namespace flecsi
