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

#include <type_traits>
#include <vector>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <legion/arrays.h>

#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/topology/mesh_topology.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/topology/set_topology.h>
#include <flecsi/utils/tuple_walker.h>

namespace flecsi {
namespace execution {

/*!
  The init_handles_t type can be called to walk task args after task
  launch. This allows us to map physical regions to internal handle
  buffers/accessors.

  @ingroup execution
 */

struct init_handles_t : public utils::tuple_walker__<init_handles_t> {

  /*!
    Construct an init_handles_t instance.
  
    @param runtime The Legion task runtime.
    @param context The Legion task runtime context.
   */

  init_handles_t(
      Legion::Runtime * runtime,
      Legion::Context & context,
      const std::vector<Legion::PhysicalRegion> & regions)
      : runtime(runtime), context(context), regions(regions), region(0) {
  } // init_handles

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

    constexpr size_t num_regions = 3;

    h.context = context;
    h.runtime = runtime;

    Legion::PhysicalRegion prs[num_regions];
    T * data[num_regions];
    size_t sizes[num_regions];
    h.combined_size = 0;

    size_t permissions[] = {EXCLUSIVE_PERMISSIONS, SHARED_PERMISSIONS,
                            GHOST_PERMISSIONS};

    // Get sizes, physical regions, and raw rect buffer for each of ex/sh/gh
    for (size_t r = 0; r < num_regions; ++r) {
      if (permissions[r] == size_t(reserved)) {
        data[r] = nullptr;
        sizes[r] = 0;
        prs[r] = Legion::PhysicalRegion();

        clog(error) << "reserved permissions mode used on region " << r
                    << std::endl;
      } else {
        prs[r] = regions[region + r];
        Legion::LogicalRegion lr = prs[r].get_logical_region();
        Legion::IndexSpace is = lr.get_index_space();

        auto ac = prs[r].get_field_accessor(h.fid).template typeify<T>();

        Legion::Domain domain = runtime->get_index_space_domain(context, is);

        LegionRuntime::Arrays::Rect<2> dr = domain.get_rect<2>();
        LegionRuntime::Arrays::Rect<2> sr;
        LegionRuntime::Accessor::ByteOffset bo[2];
        data[r] = ac.template raw_rect_ptr<2>(dr, sr, bo);
        // data[r] += bo[1];
        sizes[r] = sr.hi[1] - sr.lo[1] + 1;
        h.combined_size += sizes[r];
      } // if
    } // for

    // region += num_regions;

#ifndef MAPPER_COMPACTION
      // Create the concatenated buffer E+S+G
      h.combined_data = new T[h.combined_size];

      // Set additional fields needed by the data handle/accessor
      // and copy into the combined buffer. Note that exclusive_data, etc.
      // aliases the combined buffer for its respective region.
      size_t pos{0};

      for (size_t r{0}; r < num_regions; ++r) {
        switch (r) {
          case 0: // Exclusive
            h.exclusive_size = sizes[r];
            h.exclusive_pr = prs[r];
            h.exclusive_data =
                h.exclusive_size == 0 ? nullptr : h.combined_data + pos;
            h.exclusive_buf = data[r];
            h.exclusive_priv = EXCLUSIVE_PERMISSIONS;
            break;
          case 1: // Shared
            h.shared_size = sizes[r];
            h.shared_pr = prs[r];
            h.shared_data =
                h.shared_size == 0 ? nullptr : h.combined_data + pos;
            h.shared_buf = data[r];
            h.shared_priv = SHARED_PERMISSIONS;
            break;
          case 2: // Ghost
            h.ghost_size = sizes[r];
            h.ghost_pr = prs[r];
            h.ghost_data = h.ghost_size == 0 ? nullptr : h.combined_data + pos;
            h.ghost_buf = data[r];
            h.ghost_priv = GHOST_PERMISSIONS;
            break;
          default:
            clog_fatal("invalid permissions case");
        } // switch

        std::memcpy(h.combined_data + pos, data[r], sizes[r] * sizeof(T));
        pos += sizes[r];
      } // for
#ifdef COMPACTED_STORAGE_SORT
      h.combined_data_sort = new T[h.combined_size];

      context_t & context_ = context_t::instance();

      auto & gis_to_cis = context_.gis_to_cis_map(h.index_space);

      size_t indx = 0;
      for (auto & citr : gis_to_cis) {
        size_t c = citr.second;
        assert(c < h.combined_size);
        h.combined_data_sort[indx] = h.combined_data[c];
        indx++;
      }

#endif

#else
      {
        Legion::LogicalRegion lr = regions[region].get_logical_region();
        Legion::IndexSpace is = lr.get_index_space();

        // we need to get Rect for the parent index space in purpose to loop
        // over  compacted physical instance
        Legion::IndexPartition parent_ip =
            runtime->get_parent_index_partition(is);
        Legion::IndexSpace parent_is =
            runtime->get_parent_index_space(parent_ip);

        Legion::Domain parent_dom =
            runtime->get_index_space_domain(context, parent_is);
        LegionRuntime::Arrays::Rect<2> parent_rect = parent_dom.get_rect<2>();

        LegionRuntime::Arrays::Rect<2> sr;
        LegionRuntime::Accessor::ByteOffset bo[2];

        // get an accessor to the first element in exclusive LR:
        auto ac = prs[0].get_field_accessor(h.fid).template typeify<T>();
        h.combined_data = ac.template raw_rect_ptr<2>(parent_rect, sr, bo);
        // h.combined_data += bo[1];
      } // scope

      size_t pos{0};
      for (size_t r{0}; r < num_regions; ++r) {
        switch (r) {
          case 0: // Exclusive
            h.exclusive_size = sizes[r];
            h.exclusive_pr = prs[r];
            h.exclusive_data =
                h.exclusive_size == 0 ? nullptr : h.combined_data;
            h.exclusive_buf = data[r];
            h.exclusive_priv = EXCLUSIVE_PERMISSIONS;
            break;
          case 1: // Shared
            h.shared_size = sizes[r];
            h.shared_pr = prs[r];
            h.shared_data =
                h.shared_size == 0 ? nullptr : h.combined_data + pos;
            h.shared_buf = data[r];
            h.shared_priv = SHARED_PERMISSIONS;
            break;
          case 2: // Ghost
            h.ghost_size = sizes[r];
            h.ghost_pr = prs[r];
            h.ghost_data = h.ghost_size == 0 ? nullptr : h.combined_data + pos;
            h.ghost_buf = data[r];
            h.ghost_priv = GHOST_PERMISSIONS;
            break;
          default:
            clog_fatal("invalid permissions case");
        } // switch

        pos += sizes[r];
      } // for
#endif

    region += num_regions;

  } // handle

  template<
      typename T,
      size_t PERMISSIONS>
  void handle(global_accessor__<
              T,
              PERMISSIONS> &a) {
    auto & h = a.handle;

    constexpr size_t num_regions = 1;

    h.context = context;
    h.runtime = runtime;

    Legion::PhysicalRegion prs[num_regions];
    T * data[num_regions];
    size_t sizes[num_regions];
    h.combined_size = 0;

    size_t permissions[] = {PERMISSIONS};

    // Get sizes, physical regions, and raw rect buffer for each of ex/sh/gh
    for (size_t r = 0; r < num_regions; ++r) {
      if (permissions[r] == size_t(reserved)) {
        data[r] = nullptr;
        sizes[r] = 0;
        prs[r] = Legion::PhysicalRegion();

        clog(error) << "reserved permissions mode used on region " << r
                    << std::endl;
      } else {
        prs[r] = regions[region + r];
        Legion::LogicalRegion lr = prs[r].get_logical_region();
        Legion::IndexSpace is = lr.get_index_space();

        auto ac = prs[r].get_field_accessor(h.fid).template typeify<T>();

        Legion::Domain domain = runtime->get_index_space_domain(context, is);

        LegionRuntime::Arrays::Rect<1> dr = domain.get_rect<1>();
        LegionRuntime::Arrays::Rect<1> sr;
        LegionRuntime::Accessor::ByteOffset bo[2];
        data[r] = ac.template raw_rect_ptr<1>(dr, sr, bo);
        // data[r] += bo[1];
        sizes[r] = sr.hi[1] - sr.lo[1] + 1;
        h.combined_size += sizes[r];
        h.combined_data = data[r];
        h.color_priv = PERMISSIONS;
        h.color_buf = data[r];
        //h.color_size = sizes[r];
        h.color_pr = prs[r];
      } // if
    } // for
   region += num_regions;
 }//global_handle

  template<
      typename T,
      size_t PERMISSIONS>
  void handle(color_accessor__<
              T,
              PERMISSIONS> &a) {
    auto & h = a.handle;

    constexpr size_t num_regions = 1;

    h.context = context;
    h.runtime = runtime;

    Legion::PhysicalRegion prs[num_regions];
    T * data[num_regions];
    size_t sizes[num_regions];
    h.combined_size = 0;

    size_t permissions[] = {PERMISSIONS};

    // Get sizes, physical regions, and raw rect buffer for each of ex/sh/gh
    for (size_t r = 0; r < num_regions; ++r) {
      if (permissions[r] == size_t(reserved)) {
        data[r] = nullptr;
        sizes[r] = 0;
        prs[r] = Legion::PhysicalRegion();

        clog(error) << "reserved permissions mode used on region " << r
                    << std::endl;
      } else {
        prs[r] = regions[region + r];
        Legion::LogicalRegion lr = prs[r].get_logical_region();
        Legion::IndexSpace is = lr.get_index_space();

        auto ac = prs[r].get_field_accessor(h.fid).template typeify<T>();

        Legion::Domain domain = runtime->get_index_space_domain(context, is);

        LegionRuntime::Arrays::Rect<1> dr = domain.get_rect<1>();
        LegionRuntime::Arrays::Rect<1> sr;
        LegionRuntime::Accessor::ByteOffset bo[2];
        data[r] = ac.template raw_rect_ptr<1>(dr, sr, bo);
        // data[r] += bo[1];
        sizes[r] = sr.hi[1] - sr.lo[1] + 1;
        h.combined_size += sizes[r];
        h.combined_data = data[r];
        h.color_priv = PERMISSIONS;
        h.color_buf = data[r];
        //h.color_size = sizes[r];
        h.color_pr = prs[r];
      } // if
    } // for
   region += num_regions;
 }//color_handle



  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
      std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle__<T, PERMISSIONS> & h) {
    auto & context_ = context_t::instance();

    auto storage = h.set_storage(new typename T::storage_t);

    //------------------------------------------------------------------------//
    // Mapping entity data from Legion and initializing mesh storage.
    //------------------------------------------------------------------------//

    std::unordered_map<size_t, size_t> region_map;

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    LegionRuntime::Arrays::Rect<2> dr;
    LegionRuntime::Arrays::Rect<2> sr;
    LegionRuntime::Accessor::ByteOffset bo[2];

    for (size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      region_map[ent.index_space] = region;

      Legion::LogicalRegion lr = regions[region].get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();

      auto ac = regions[region].get_field_accessor(ent.fid);

      Legion::Domain d = runtime->get_index_space_domain(context, is);

      dr = d.get_rect<2>();

      auto ents_raw =
          static_cast<uint8_t *>(ac.template raw_rect_ptr<2>(dr, sr, bo));
      auto ents = reinterpret_cast<topology::mesh_entity_base_ *>(ents_raw);

      size_t num_ents = sr.hi[1] - sr.lo[1] + 1;

      auto ac2 = regions[region]
                     .get_field_accessor(ent.id_fid)
                     .template typeify<utils::id_t>();
      auto ids = ac2.template raw_rect_ptr<2>(dr, sr, bo);

      storage->init_entities(
          ent.domain, ent.dim, ents, ids, ent.size, num_ents, ent.num_exclusive,
          ent.num_shared, ent.num_ghost, _read);

      ++region;
    } // for

    //------------------------------------------------------------------------//
    // Mapping adjacency data from Legion and initializing mesh storage.
    //------------------------------------------------------------------------//

    for (size_t i{0}; i < h.num_handle_adjacencies; ++i) {
      data_client_handle_adjacency_t & adj = h.handle_adjacencies[i];

      Legion::PhysicalRegion pr = regions[region_map[adj.from_index_space]];
      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();

      auto ac = pr.get_field_accessor(adj.offset_fid)
                    .template typeify<utils::offset_t>();

      Legion::Domain d = runtime->get_index_space_domain(context, is);

      dr = d.get_rect<2>();

      utils::offset_t * offsets = ac.template raw_rect_ptr<2>(dr, sr, bo);

      size_t num_offsets = sr.hi[1] - sr.lo[1] + 1;

      // Store these for translation to CRS
      adj.num_offsets = num_offsets;

      clog(trace) << "num_offsets: " << num_offsets << std::endl;

      lr = regions[region].get_logical_region();
      is = lr.get_index_space();

      auto ac3 = regions[region]
                     .get_field_accessor(adj.index_fid)
                     .template typeify<utils::id_t>();

      d = runtime->get_index_space_domain(context, is);

      dr = d.get_rect<2>();

      utils::id_t * indices = ac3.template raw_rect_ptr<2>(dr, sr, bo);

      size_t num_indices = sr.hi[1] - sr.lo[1] + 1;

      adj.num_indices = num_indices;

      storage->init_connectivity(
          adj.from_domain, adj.to_domain, adj.from_dim, adj.to_dim, offsets,
          num_offsets, indices, num_indices, _read);

      ++region;
    } // for

    if (!_read) {
      h.initialize_storage();
    }
  }

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
      std::is_base_of<topology::set_topology_base_t, T>::value>
  handle(data_client_handle__<T, PERMISSIONS> & h) {
    auto & context_ = context_t::instance();

    auto storage = h.set_storage(new typename T::storage_t);

    //------------------------------------------------------------------------//
    // Mapping entity data from Legion and initializing set storage.
    //------------------------------------------------------------------------//

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    for (size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      Legion::PhysicalRegion pr = regions[region];
      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();

      auto ac = pr.get_field_accessor(ent.fid);
      Legion::Domain domain = runtime->get_index_space_domain(context, is);
      LegionRuntime::Arrays::Rect<1> r = domain.get_rect<1>();
      LegionRuntime::Arrays::Rect<1> sr;
      LegionRuntime::Accessor::ByteOffset bo[1];

      auto ents_raw =
          static_cast<uint8_t *>(ac.template raw_rect_ptr<1>(r, sr, bo));
      auto ents = reinterpret_cast<topology::set_entity_t *>(ents_raw);

      size_t num_ents = sr.hi[0] - sr.lo[0] + 1;

      storage->init_entities(ent.index_space, ents, ent.size, num_ents, _read);

      ++region;
    } // for
  } // handle

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
  const std::vector<Legion::PhysicalRegion> & regions;
  size_t region;
}; // struct init_handles_t

} // namespace execution
} // namespace flecsi
