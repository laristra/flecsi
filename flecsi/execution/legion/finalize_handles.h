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

namespace flecsi {
namespace execution {

/*!
  FIXME
  @ingroup execution
 */
struct finalize_handles_t : public utils::tuple_walker__<finalize_handles_t> {

  /*!
    FIXME
     @ingroup execution
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
              GHOST_PERMISSIONS> & a) {} // handle

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
    using entry_value_t = typename mutator_handle__<T>::entry_value_t;
    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & h = a.handle;
    auto md = static_cast<sparse_field_data_t*>(h.metadata);
    
    std::memcpy(h.entries_data[0], h.entries,
                md->num_exclusive_filled * sizeof(entry_value_t));

    std::memcpy(h.entries_data[1],
                h.entries + md->reserve,
                md->num_shared * sizeof(entry_value_t) * 
                md->max_entries_per_index);
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
    using entry_value_t = typename mutator_handle__<T>::entry_value_t;
    using commit_info_t = typename mutator_handle__<T>::commit_info_t;
    using offset_t = data::sparse_data_offset_t;
    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & h = m.h_;

    entry_value_t *entries =
      reinterpret_cast<entry_value_t *>(h.entries);

    commit_info_t ci;
    ci.offsets = h.offsets;
    ci.entries[0] = entries;
    ci.entries[1] = entries + h.reserve;
    ci.entries[2] =
      ci.entries[1] + h.num_shared() * h.max_entries_per_index();

    auto md = static_cast<sparse_field_data_t*>(h.metadata);

    md->num_exclusive_filled = h.commit(&ci);

    std::memcpy(h.offsets_data[0], h.offsets,
                h.num_exclusive() * sizeof(offset_t));
    
    std::memcpy(h.offsets_data[1], h.offsets + h.num_exclusive(),
                h.num_shared() * sizeof(offset_t));

    if(!md->initialized){
      std::memcpy(h.offsets_data[2],
                  h.offsets + h.num_exclusive() + h.num_shared(),
                  h.num_ghost() * sizeof(offset_t));      
    }

    std::memcpy(h.entries_data[0], h.entries,
                md->num_exclusive_filled * sizeof(entry_value_t));
    
    std::memcpy(h.entries_data[1],
                h.entries + h.reserve * sizeof(entry_value_t),
                h.num_shared() * sizeof(entry_value_t) * 
                h.max_entries_per_index());

    md->initialized = true;
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
     The finalize_handles_t type can be called to walk task args after task
     execution. This allows us to free memory allocated during the task.

     @ingroup execution
   */

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
      std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle__<T, PERMISSIONS> & h) {

    if (PERMISSIONS == wo || PERMISSIONS == rw) {
      auto & context_ = context_t::instance();
      auto & ssm = context_.index_subspace_info();

      for (size_t i{0}; i < h.num_index_subspaces; ++i) {
        data_client_handle_index_subspace_t & iss = h.handle_index_subspaces[i];

        auto itr = ssm.find(iss.index_subspace);
        clog_assert(itr != ssm.end(), "invalid index subspace");
        context_t::index_subspace_info_t & si = itr->second;

        clog_assert(si.size == 0, "index subspace size already set");
        si.size = h.get_index_subspace_size_(iss.index_subspace);
      }
    }

    h.delete_storage();
  } // handle

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
      !std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle__<T, PERMISSIONS> & h) {
    h.delete_storage();
  } // handle

  /*!
    If this is not a data handle, then simply skip it.
   */

  template<typename T>
  static typename std::enable_if_t<
      !std::is_base_of<dense_accessor_base_t, T>::value>
  handle(T &) {} // handle

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi
