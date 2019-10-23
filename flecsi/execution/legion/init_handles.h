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

#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
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

struct init_handles_t : public flecsi::utils::tuple_walker_u<init_handles_t> {

  /*!
    Construct an init_handles_t instance.

    @param runtime The Legion task runtime.
    @param context The Legion task runtime context.
   */

  init_handles_t(Legion::Runtime * runtime,
    Legion::Context & context,
    const std::vector<Legion::PhysicalRegion> & regions,
    const std::vector<Legion::Future> & futures)
    : runtime(runtime), context(context), regions(regions), futures(futures),
      region(0), future_id(0) {} // init_handles

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(dense_accessor_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    constexpr size_t num_regions = 3;

    h.context = context;
    h.runtime = runtime;
    h.combined_size = 0;

    const int my_color = runtime->find_local_MPI_rank();

    size_t permissions[] = {
      EXCLUSIVE_PERMISSIONS, SHARED_PERMISSIONS, GHOST_PERMISSIONS};

    Legion::LogicalRegion lr = regions[region].get_logical_region();
    Legion::IndexSpace is = lr.get_index_space();
    Legion::Domain dom = runtime->get_index_space_domain(context, is);
    // we need to get Rect for the parent index space in purpose to loop
    // over  compacted physical instance

    Legion::Domain::DomainPointIterator itr(dom);
    LegionRuntime::Arrays::Rect<2> rect = dom.get_rect<2>();
    const Legion::UnsafeFieldAccessor<T, 2, Legion::coord_t,
      Realm::AffineAccessor<T, 2, Legion::coord_t>>
      ac(regions[region], h.fid, sizeof(T));

    T * ac_ptr = (T *)(ac.ptr(itr.p));

    // get an accessor to the first element in exclusive LR:
    h.combined_data = ac_ptr;
    // Exclusive
    h.exclusive_size = rect.hi[1] - rect.lo[1] + 1;
    h.exclusive_data = h.exclusive_size == 0 ? nullptr : h.combined_data;
    // Shared
    LegionRuntime::Arrays::Rect<2> rect_sh =
      runtime
        ->get_index_space_domain(
          context, regions[region + 1].get_logical_region().get_index_space())
        .get_rect<2>();
    h.shared_size = rect_sh.hi[1] - rect_sh.lo[1] + 1;
    h.shared_data =
      h.shared_size == 0 ? nullptr : h.combined_data + h.exclusive_size;
    // Ghost
    LegionRuntime::Arrays::Rect<2> rect_gh =
      runtime
        ->get_index_space_domain(
          context, regions[region + 2].get_logical_region().get_index_space())
        .get_rect<2>();
    h.ghost_size = rect_gh.hi[1] - rect_gh.lo[1] + 1;
    h.ghost_data = h.ghost_size == 0
                     ? nullptr
                     : h.combined_data + h.exclusive_size + h.shared_size;

    h.combined_size = h.exclusive_size + h.shared_size + h.ghost_size;

    region += num_regions;
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor_u<T, PERMISSIONS> & a) {
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
    for(size_t r = 0; r < num_regions; ++r) {
      if(permissions[r] == size_t(reserved)) {
        clog(error) << "reserved permissions mode used on region " << r
                    << std::endl;
      }
      else {
        Legion::Domain dom = runtime->get_index_space_domain(
          context, regions[region + r].get_logical_region().get_index_space());
        Legion::Domain::DomainPointIterator itr(dom);
        LegionRuntime::Arrays::Rect<1> dr = dom.get_rect<1>();
        const Legion::UnsafeFieldAccessor<T, 1, Legion::coord_t,
          Realm::AffineAccessor<T, 1, Legion::coord_t>>
          ac(regions[r], h.fid, sizeof(T));

        T * ac_ptr = (T *)(ac.ptr(itr.p));
        data[r] = ac_ptr;
        sizes[r] = dr.hi[1] - dr.lo[1] + 1;
        h.combined_size += sizes[r];
        h.combined_data = data[r];
        h.color_priv = PERMISSIONS;
        h.color_buf = data[r];
        // h.color_size = sizes[r];
        h.color_pr = prs[r];
      } // if
    } // for
    region += num_regions;
  } // global_handle

  template<typename T, size_t PERMISSIONS>
  void handle(color_accessor_u<T, PERMISSIONS> & a) {
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
    for(size_t r = 0; r < num_regions; ++r) {
      if(permissions[r] == size_t(reserved)) {
        clog(error) << "reserved permissions mode used on region " << r
                    << std::endl;
      }
      else {
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
        // h.color_size = sizes[r];
        h.color_pr = prs[r];
      } // if
    } // for
    region += num_regions;
  } // color_handle

  /*!
   Initialize arguments for future handle
   */
  template<typename T, launch_type_t launch>
  void handle(legion_future_u<T, launch> & h) {
    h.data_ = Legion::Future(futures[future_id]).get_result<T>();
    future_id++;
  } // handle

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> & h) {
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

    for(size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      region_map[ent.index_space] = region;

      Legion::LogicalRegion lr = regions[region].get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();

      const Legion::UnsafeFieldAccessor<char, 2, Legion::coord_t,
        Realm::AffineAccessor<char, 2, Legion::coord_t>>
        ac(regions[region], ent.fid, ent.fid_size);

      Legion::Domain d = runtime->get_index_space_domain(context, is);
      dr = d.get_rect<2>();
      Legion::Domain::DomainPointIterator itr(d);

      char * ac_ptr = (char *)(ac.ptr(itr.p));
      auto ents = reinterpret_cast<topology::mesh_entity_base_ *>(ac_ptr);

      size_t num_ents = dr.hi[1] - dr.lo[1] + 1;

      const Legion::UnsafeFieldAccessor<utils::id_t, 2, Legion::coord_t,
        Realm::AffineAccessor<utils::id_t, 2, Legion::coord_t>>
        ac2(regions[region], ent.id_fid, sizeof(utils::id_t));
      // get an accessor to the first element in exclusive LR:
      utils::id_t * ac2_ptr = (utils::id_t *)(ac2.ptr(itr.p));

      auto ids = ac2_ptr;
      // calculating exclusive, shared and ghost sizes fro the entity
      const auto & coloring = context_.coloring(ent.index_space);
      ent.num_exclusive = coloring.exclusive.size();
      ent.num_shared = coloring.shared.size();
      ent.num_ghost = coloring.ghost.size();

      storage->init_entities(ent.domain, ent.dim, ents, ids, ent.size, num_ents,
        ent.num_exclusive, ent.num_shared, ent.num_ghost, _read);

      ++region;
    } // for

    //------------------------------------------------------------------------//
    // Mapping adjacency data from Legion and initializing mesh storage.
    //------------------------------------------------------------------------//

    for(size_t i{0}; i < h.num_handle_adjacencies; ++i) {
      data_client_handle_adjacency_t & adj = h.handle_adjacencies[i];

      Legion::PhysicalRegion pr = regions[region_map[adj.from_index_space]];
      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();
      Legion::Domain d = runtime->get_index_space_domain(context, is);
      Legion::Domain::DomainPointIterator itr(d);

      const Legion::UnsafeFieldAccessor<utils::offset_t, 2, Legion::coord_t,
        Realm::AffineAccessor<utils::offset_t, 2, Legion::coord_t>>
        ac(regions[region_map[adj.from_index_space]], adj.offset_fid,
          sizeof(utils::offset_t));

      utils::offset_t * offsets = (utils::offset_t *)(ac.ptr(itr.p));
      dr = d.get_rect<2>();

      size_t num_offsets = dr.hi[1] - dr.lo[1] + 1;

      // Store these for translation to CRS
      adj.num_offsets = num_offsets;

      clog(trace) << "num_offsets: " << num_offsets << std::endl;

      lr = regions[region].get_logical_region();
      is = lr.get_index_space();
      const Legion::UnsafeFieldAccessor<utils::id_t, 2, Legion::coord_t,
        Realm::AffineAccessor<utils::id_t, 2, Legion::coord_t>>
        ac3(regions[region], adj.index_fid, sizeof(utils::id_t));

      d = runtime->get_index_space_domain(context, is);

      dr = d.get_rect<2>();

      utils::id_t * indices = (utils::id_t *)(ac3.ptr(itr.p));

      size_t num_indices = dr.hi[1] - dr.lo[1] + 1;

      adj.num_indices = num_indices;

      storage->init_connectivity(adj.from_domain, adj.to_domain, adj.from_dim,
        adj.to_dim, offsets, num_offsets, indices, num_indices, _read);

      ++region;
    } // for

    for(size_t i{0}; i < h.num_index_subspaces; ++i) {
      data_client_handle_index_subspace_t & iss = h.handle_index_subspaces[i];

      Legion::PhysicalRegion pr = regions[region];
      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();
      Legion::Domain d = runtime->get_index_space_domain(context, is);
      const Legion::UnsafeFieldAccessor<utils::id_t, 2, Legion::coord_t,
        Realm::AffineAccessor<utils::id_t, 2, Legion::coord_t>>
        ac(regions[region], iss.index_fid, sizeof(utils::id_t));
      Legion::Domain::DomainPointIterator itr(d);

      dr = d.get_rect<2>();

      utils::id_t * ids = (utils::id_t *)(ac.ptr(itr.p));

      size_t num_indices = dr.hi[1] - dr.lo[1] + 1;

      storage->init_index_subspace(iss.index_space, iss.index_subspace,
        iss.domain, iss.dim, ids, num_indices, _read);

      ++region;
    }

    if(!_read) {
      h.initialize_storage();
    }
  }

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::set_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> & h) {
    auto & context_ = context_t::instance();

    auto storage = h.set_storage(new typename T::storage_t);

    //------------------------------------------------------------------------//
    // Mapping entity data from Legion and initializing set storage.
    //------------------------------------------------------------------------//

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    for(size_t i{0}; i < h.num_handle_entities; ++i) {
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

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(ragged_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    constexpr size_t num_regions = 3;

    using value_t = T;
    using vector_t = data::row_vector_u<T>;
    using sparse_field_data_t = context_t::sparse_field_data_t;

    sparse_field_data_t * md;

    {
      Legion::PhysicalRegion pr = regions[region];

      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();

      auto ac =
        pr.get_field_accessor(h.fid).template typeify<sparse_field_data_t>();

      Legion::Domain domain = runtime->get_index_space_domain(context, is);

      LegionRuntime::Arrays::Rect<2> dr = domain.get_rect<2>();
      LegionRuntime::Arrays::Rect<2> sr;
      LegionRuntime::Accessor::ByteOffset bo[2];
      md = ac.template raw_rect_ptr<2>(dr, sr, bo);
      h.metadata = md;

      h.init(md->num_exclusive, md->num_shared, md->num_ghost);
    }

    ++region;

    Legion::LogicalRegion lr_s = regions[region].get_logical_region();
    Legion::IndexSpace is_s = lr_s.get_index_space();
    auto ac =
      regions[region].get_field_accessor(h.fid).template typeify<vector_t>();
    Legion::Domain domain_s = runtime->get_index_space_domain(context, is_s);
    LegionRuntime::Arrays::Rect<2> dr = domain_s.get_rect<2>();
    LegionRuntime::Arrays::Rect<2> sr;
    LegionRuntime::Accessor::ByteOffset bo[2];
    h.rows = ac.template raw_rect_ptr<2>(dr, sr, bo);

    region += num_regions;

  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(sparse_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    handle(a.ragged);
  } // handle

  template<typename T>
  void handle(ragged_mutator<T> & m) {
    auto & h = m.handle;

    constexpr size_t num_regions = 3;

    using value_t = T;
    using vector_t = data::row_vector_u<T>;
    using sparse_field_data_t = context_t::sparse_field_data_t;

    sparse_field_data_t * md;

    {
      Legion::PhysicalRegion pr = regions[region];

      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();

      auto ac =
        pr.get_field_accessor(h.fid).template typeify<sparse_field_data_t>();

      Legion::Domain domain = runtime->get_index_space_domain(context, is);

      LegionRuntime::Arrays::Rect<2> dr = domain.get_rect<2>();
      LegionRuntime::Arrays::Rect<2> sr;
      LegionRuntime::Accessor::ByteOffset bo[2];
      md = ac.template raw_rect_ptr<2>(dr, sr, bo);

      h.metadata = md;

      h.init(md->num_exclusive, md->num_shared,
        md->num_ghost); //, md->max_entries_per_index, h.slots);
    }

    ++region;

    Legion::LogicalRegion lr_s = regions[region].get_logical_region();
    Legion::IndexSpace is_s = lr_s.get_index_space();
    auto ac =
      regions[region].get_field_accessor(h.fid).template typeify<vector_t>();
    Legion::Domain domain_s = runtime->get_index_space_domain(context, is_s);
    LegionRuntime::Arrays::Rect<2> dr = domain_s.get_rect<2>();
    LegionRuntime::Arrays::Rect<2> sr;
    LegionRuntime::Accessor::ByteOffset bo[2];
    h.rows = ac.template raw_rect_ptr<2>(dr, sr, bo);

    region += num_regions;

  } // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }

  /*!
   Handle individual list items
   */
  template<typename T,
    std::size_t N,
    template<typename, std::size_t>
    typename Container,
    typename =
      std::enable_if_t<std::is_base_of<data::data_reference_base_t, T>::value>>
  void handle(Container<T, N> & list) {
    for(auto & item : list)
      handle(item);
  }

  Legion::Runtime * runtime;
  Legion::Context & context;
  const std::vector<Legion::PhysicalRegion> & regions;
  size_t region;
  const std::vector<Legion::Future> & futures;
  size_t future_id;
}; // struct init_handles_t

} // namespace execution
} // namespace flecsi
