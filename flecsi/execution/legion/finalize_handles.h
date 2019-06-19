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

#include <flecsi/data/common/data_reference.h>
#include <flecsi/utils/tuple_walker.h>

#ifdef ENABLE_CALIPER
#include <caliper/cali.h>
#endif
namespace flecsi {
namespace execution {

/*!
  FIXME
  @ingroup execution
 */

struct finalize_handles_t
  : public flecsi::utils::tuple_walker_u<finalize_handles_t> {

  /*!
    FIXME
     @ingroup execution
   */

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(dense_accessor_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
#ifndef MAPPER_COMPACTION
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("dense_accessor");
#endif
    auto & h = a.handle;

    if((EXCLUSIVE_PERMISSIONS == rw) || (EXCLUSIVE_PERMISSIONS == wo))
      std::memcpy(
        h.exclusive_buf, h.exclusive_data, h.exclusive_size * sizeof(T));

    if((SHARED_PERMISSIONS == rw) || (SHARED_PERMISSIONS == wo))
      std::memcpy(h.shared_buf, h.shared_data, h.shared_size * sizeof(T));
#ifdef ENABLE_CALIPER
CALI_MARK_END("dense_accessor");
#endif
#endif
  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(ragged_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("ragged_accessor");
#endif
    using value_t = T;
    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & h = a.handle;
    auto md = static_cast<sparse_field_data_t *>(h.metadata);

#ifndef MAPPER_COMPACTION
    std::memcpy(
      h.entries_data[0], h.entries, md->num_exclusive_filled * sizeof(value_t));

    std::memcpy(h.entries_data[1], h.entries + md->reserve,
      md->num_shared * sizeof(value_t) * md->max_entries_per_index);
#endif
#ifdef ENABLE_CALIPER
CALI_MARK_END("ragged_accessor");
#endif
  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(sparse_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("sparse_accessor");
#endif
    using base_t = typename sparse_accessor<T, EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS, GHOST_PERMISSIONS>::base_t;
    handle(static_cast<base_t &>(a));
#ifdef ENABLE_CALIPER
CALI_MARK_END("sparse_accessor");
#endif
  } // handle

  template<typename T>
  void handle(ragged_mutator<T> & m) {

#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("ragged_mutator");
#endif
    using value_t = T;
    using commit_info_t = typename mutator_handle_u<T>::commit_info_t;
    using offset_t = data::sparse_data_offset_t;
    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & h = m.h_;

    value_t * entries = reinterpret_cast<value_t *>(h.entries);

    commit_info_t ci;
    ci.offsets = h.offsets;
    ci.entries[0] = entries;
    ci.entries[1] = entries + h.reserve;
    ci.entries[2] = ci.entries[1] + h.num_shared() * h.max_entries_per_index();

    auto md = static_cast<sparse_field_data_t *>(h.metadata);

    md->num_exclusive_filled = h.commit(&ci);

#ifndef MAPPER_COMPACTION
    std::memcpy(
      h.offsets_data[0], h.offsets, h.num_exclusive() * sizeof(offset_t));

    std::memcpy(h.offsets_data[1], h.offsets + h.num_exclusive(),
      h.num_shared() * sizeof(offset_t));

    if(!md->initialized) {
      std::memcpy(h.offsets_data[2],
        h.offsets + h.num_exclusive() + h.num_shared(),
        h.num_ghost() * sizeof(offset_t));
    }

    std::memcpy(
      h.entries_data[0], h.entries, md->num_exclusive_filled * sizeof(value_t));

    std::memcpy(h.entries_data[1], h.entries + h.reserve * sizeof(value_t),
      h.num_shared() * sizeof(value_t) * h.max_entries_per_index());
#endif

    md->initialized = true;
#ifdef ENABLE_CALIPER
CALI_MARK_END("ragged_mutator");
#endif
  } // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("sparse_mutator");
#endif
    using base_t = typename sparse_mutator<T>::base_t;
    handle(static_cast<base_t &>(m));
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("sparse_mutator");
#endif
  }

  /*!
     The finalize_handles_t type can be called to walk task args after task
     execution. This allows us to free memory allocated during the task.

     @ingroup execution
   */

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> & h) {

#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("data_client_handle_u");
#endif

    if(PERMISSIONS == wo || PERMISSIONS == rw) {
      auto & context_ = context_t::instance();
      auto & ssm = context_.index_subspace_info();

      for(size_t i{0}; i < h.num_index_subspaces; ++i) {
        data_client_handle_index_subspace_t & iss = h.handle_index_subspaces[i];

        auto itr = ssm.find(iss.index_subspace);
        clog_assert(itr != ssm.end(), "invalid index subspace");
        context_t::index_subspace_info_t & si = itr->second;

        clog_assert(si.size == 0, "index subspace size already set");
        si.size = h.get_index_subspace_size_(iss.index_subspace);
      }
    }
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("data_client_handle_u_delete_storage");
#endif
    h.delete_storage();
#ifdef ENABLE_CALIPER
CALI_MARK_END("data_client_handle_u_delete_storage");
CALI_MARK_END("data_client_handle_u");
#endif
  } // handle

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    !std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> & h) {
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("data_client_handle_delete_storage");
#endif
    h.delete_storage();
#ifdef ENABLE_CALIPER
CALI_MARK_END("data_client_handle_delete_storage");
#endif
  } // handle

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
#ifdef ENABLE_CALIPER
CALI_MARK_BEGIN("container");
#endif
    for(auto & item : list)
      handle(item);
#ifdef ENABLE_CALIPER
CALI_MARK_END("container");
#endif
  }

  /*!
    If this is not a data handle, then simply skip it.
   */

  template<typename T, launch_type_t launch>
  void handle(legion_future_u<T, launch> & h) {}

  template<typename T>
  static
    typename std::enable_if_t<!std::is_base_of<dense_accessor_base_t, T>::value>
    handle(T &) {} // handle

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi
