/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <vector>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/execution/common/execution_state.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/topology/set_types.h>
#include <flecsi/utils/tuple_walker.h>

namespace flecsi {
namespace execution {

/*!
  The init_args_t type can be called to walk task args before the
  task launcher is created. This allows us to gather region requirements
  and to set state on the associated data handles \em before Legion gets
  the task arguments tuple.

  @ingroup execution
*/

struct init_args_t : public utils::tuple_walker__<init_args_t> {

  /*!
    Construct an init_args_t instance.

    @param runtime The Legion task runtime.
    @param context The Legion task runtime context.
   */
  init_args_t(Legion::Runtime * runtime, Legion::Context & context)
      : runtime(runtime), context(context) {} // init_args

  /*!
    Convert the template privileges to proper Legion privileges.

    @param mode privilege
   */

  static Legion::PrivilegeMode privilege_mode(size_t mode) {
    switch (mode) {
      case size_t(reserved):
        return NO_ACCESS;
      case size_t(ro):
        return READ_ONLY;
      case size_t(wo):
        return WRITE_DISCARD;
      case size_t(rw):
        return READ_WRITE;
      default:
        clog_fatal("invalid privilege mode");
    } // switch
    // should never get here, but this is needed
    // to avoid compiler warnings
    return NO_ACCESS;
  } // privilege_mode

  /*!
    FIXME
   */

  template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS>
  void handle(dense_accessor__<
              T,
              EXCLUSIVE_PERMISSIONS,
              SHARED_PERMISSIONS,
              GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    clog_assert(
        h.state > SPECIALIZATION_TLT_INIT,
        "accessing  data "
        "handle from specialization_tlt_init is not supported");

    Legion::MappingTagID tag = EXCLUSIVE_LR;

    Legion::RegionRequirement ex_rr(
        h.exclusive_lr, privilege_mode(EXCLUSIVE_PERMISSIONS), EXCLUSIVE,
        h.color_region, tag);
    ex_rr.add_field(h.fid);
    region_reqs.push_back(ex_rr);

    Legion::RegionRequirement sh_rr(
        h.shared_lr, privilege_mode(SHARED_PERMISSIONS), EXCLUSIVE,
        h.color_region);
    sh_rr.add_field(h.fid);
    region_reqs.push_back(sh_rr);

    Legion::RegionRequirement gh_rr(
        h.ghost_lr, privilege_mode(GHOST_PERMISSIONS), EXCLUSIVE,
        h.color_region);
    gh_rr.add_field(h.fid);
    region_reqs.push_back(gh_rr);
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor__<T, PERMISSIONS> & a) {
    auto & h = a.handle;

    if (h.state < SPECIALIZATION_SPMD_INIT) {
      Legion::RegionRequirement rr(
          h.color_region, privilege_mode(PERMISSIONS), EXCLUSIVE,
          h.color_region);
      rr.add_field(h.fid);
      region_reqs.push_back(rr);
    } else {
      clog_assert(PERMISSIONS == size_t(ro), "you are not allowed "
            "to modify global data in specialization_spmd_init or driver");
      Legion::RegionRequirement rr(h.color_region, READ_ONLY, EXCLUSIVE,
                                   h.color_region);
      rr.add_field(h.fid);
      region_reqs.push_back(rr);
    } // if
  }

  template<typename T, size_t PERMISSIONS>
  void handle(color_accessor__<T, PERMISSIONS> & a) {
    auto & h = a.handle;
    clog_assert(h.state > SPECIALIZATION_TLT_INIT, "accessing color data    \
         handle from specialization_tlt_init is not supported");
    Legion::RegionRequirement rr(
        h.color_region, privilege_mode(PERMISSIONS), EXCLUSIVE, h.color_region);
    rr.add_field(h.fid);
    region_reqs.push_back(rr);
  } // handle

  /*!
    FIXME
   */

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
      std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle__<T, PERMISSIONS> & h) {

    std::unordered_map<size_t, size_t> region_map;

    for (size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      region_map[ent.index_space] = region_reqs.size();

      Legion::RegionRequirement rr(
          ent.color_region, privilege_mode(PERMISSIONS), EXCLUSIVE,
          ent.color_region);

      Legion::IndexSpace is = ent.exclusive_region.get_index_space();

      Legion::Domain d = runtime->get_index_space_domain(context, is);

      auto dr = d.get_rect<2>();

      ent.num_exclusive = dr.hi[1] - dr.lo[1] + 1;

      is = ent.shared_region.get_index_space();

      d = runtime->get_index_space_domain(context, is);

      dr = d.get_rect<2>();

      ent.num_shared = dr.hi[1] - dr.lo[1] + 1;

      is = ent.ghost_region.get_index_space();

      d = runtime->get_index_space_domain(context, is);

      dr = d.get_rect<2>();

      ent.num_ghost = dr.hi[1] - dr.lo[1] + 1;

      rr.add_field(ent.fid);
      rr.add_field(ent.id_fid);
      region_reqs.push_back(rr);
    } // for

    for (size_t i{0}; i < h.num_handle_adjacencies; ++i) {
      data_client_handle_adjacency_t & adj = h.handle_adjacencies[i];

      region_reqs[region_map[adj.from_index_space]].add_field(adj.offset_fid);

      Legion::RegionRequirement adj_rr(
          adj.adj_region, privilege_mode(PERMISSIONS), EXCLUSIVE,
          adj.adj_region);

      adj_rr.add_field(adj.index_fid);

      region_reqs.push_back(adj_rr);
    }

    for (size_t i{0}; i < h.num_index_subspaces; ++i) {
      data_client_handle_index_subspace_t & iss = h.handle_index_subspaces[i];

      Legion::RegionRequirement iss_rr(
          iss.region, privilege_mode(PERMISSIONS), EXCLUSIVE, iss.region);

      iss_rr.add_field(iss.index_fid);

      region_reqs.push_back(iss_rr);
    }

  } // handle

  ///
  // Initialize arguments for future handle
  ///
  template<typename T, launch_type_t launch>
  void handle(legion_future__<T, launch> & h) {
    futures.push_back(std::make_shared<legion_future__<T, launch>>(h));
  }

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    sparse_accessor <
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS
    > &a
  )
  {
    auto & h = a.handle;

    Legion::MappingTagID tag = EXCLUSIVE_LR;

    Legion::RegionRequirement md_rr(
        h.metadata_color_region, READ_WRITE, EXCLUSIVE,
        h.metadata_color_region);
    md_rr.add_field(h.fid);
    region_reqs.push_back(md_rr);

    Legion::RegionRequirement ex_rr(
        h.offsets_exclusive_lr, privilege_mode(EXCLUSIVE_PERMISSIONS), EXCLUSIVE,
        h.offsets_color_region, tag);
    ex_rr.add_field(h.fid);
    region_reqs.push_back(ex_rr);

    Legion::RegionRequirement sh_rr(
        h.offsets_shared_lr, privilege_mode(SHARED_PERMISSIONS), EXCLUSIVE,
        h.offsets_color_region);
    sh_rr.add_field(h.fid);
    region_reqs.push_back(sh_rr);

    Legion::RegionRequirement gh_rr(
        h.offsets_ghost_lr, privilege_mode(GHOST_PERMISSIONS), EXCLUSIVE,
        h.offsets_color_region);
    gh_rr.add_field(h.fid);
    region_reqs.push_back(gh_rr);

    Legion::RegionRequirement ex_rr2(
        h.entries_exclusive_lr, privilege_mode(EXCLUSIVE_PERMISSIONS), EXCLUSIVE,
        h.entries_color_region, tag);
    ex_rr2.add_field(h.fid);
    region_reqs.push_back(ex_rr2);

    Legion::RegionRequirement sh_rr2(
        h.entries_shared_lr, privilege_mode(SHARED_PERMISSIONS), EXCLUSIVE,
        h.entries_color_region);
    sh_rr2.add_field(h.fid);
    region_reqs.push_back(sh_rr2);

    Legion::RegionRequirement gh_rr2(
        h.entries_ghost_lr, privilege_mode(GHOST_PERMISSIONS), EXCLUSIVE,
        h.entries_color_region);
    gh_rr2.add_field(h.fid);
    region_reqs.push_back(gh_rr2);
  }

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    ragged_accessor<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & a
  )
  {
    handle(reinterpret_cast<sparse_accessor<
      T, EXCLUSIVE_PERMISSIONS, SHARED_PERMISSIONS, GHOST_PERMISSIONS>&>(a));
  } // handle

  template<
    typename T
  >
  void
  handle(
    sparse_mutator<
    T
    > &m
  )
  {
    auto & h = m.h_;

    Legion::MappingTagID tag = EXCLUSIVE_LR;

    Legion::RegionRequirement md_rr(
        h.metadata_color_region, READ_WRITE, EXCLUSIVE,
        h.metadata_color_region);
    md_rr.add_field(h.fid);
    region_reqs.push_back(md_rr);

    Legion::RegionRequirement ex_rr(
        h.offsets_exclusive_lr, READ_WRITE, EXCLUSIVE,
        h.offsets_color_region, tag);
    ex_rr.add_field(h.fid);
    region_reqs.push_back(ex_rr);

    Legion::RegionRequirement sh_rr(
        h.offsets_shared_lr, READ_WRITE, EXCLUSIVE,
        h.offsets_color_region);
    sh_rr.add_field(h.fid);
    region_reqs.push_back(sh_rr);

    Legion::RegionRequirement gh_rr(
        h.offsets_ghost_lr, READ_WRITE, EXCLUSIVE,
        h.offsets_color_region);
    gh_rr.add_field(h.fid);
    region_reqs.push_back(gh_rr);

    Legion::RegionRequirement ex_rr2(
        h.entries_exclusive_lr, READ_WRITE, EXCLUSIVE,
        h.entries_color_region, tag);
    ex_rr2.add_field(h.fid);
    region_reqs.push_back(ex_rr2);

    Legion::RegionRequirement sh_rr2(
        h.entries_shared_lr, READ_WRITE, EXCLUSIVE,
        h.entries_color_region);
    sh_rr2.add_field(h.fid);
    region_reqs.push_back(sh_rr2);

    Legion::RegionRequirement gh_rr2(
        h.entries_ghost_lr, READ_WRITE, EXCLUSIVE,
        h.entries_color_region);
    gh_rr2.add_field(h.fid);
    region_reqs.push_back(gh_rr2);
  }

  template<
    typename T
  >
  void
  handle(
    ragged_mutator<
      T
    > & m
  )
  {
    handle(reinterpret_cast<sparse_mutator<T>&>(m));
  }

  /*!
    FIXME
   */

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
      std::is_base_of<topology::set_topology_base_t, T>::value>
  handle(data_client_handle__<T, PERMISSIONS> & h) {

    for (size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      Legion::RegionRequirement rr(
          ent.color_region, privilege_mode(PERMISSIONS), EXCLUSIVE,
          ent.color_region);
      rr.add_field(ent.fid);
      region_reqs.push_back(rr);
    } // for
  }

  //-----------------------------------------------------------------------//
  // If this is not a data handle, then simply skip it.
  //-----------------------------------------------------------------------//

  template<typename T>
  static typename std::enable_if_t<
      !std::is_base_of<dense_accessor_base_t, T>::value &&
      !std::is_base_of<data_client_handle_base_t, T>::value>
  handle(T &) {} // handle

  Legion::Runtime * runtime;
  Legion::Context & context;
  std::vector<Legion::RegionRequirement> region_reqs;
  std::vector<std::shared_ptr<future_base_t>> futures;

}; // struct init_args_t

} // namespace execution
} // namespace flecsi
