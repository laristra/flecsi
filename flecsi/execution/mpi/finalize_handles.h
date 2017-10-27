/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_finalize_handles_h
#define flecsi_execution_mpi_finalize_handles_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 19, 2017
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

struct finalize_handles_t : public utils::tuple_walker__<finalize_handles_t>
{
  //--------------------------------------------------------------------------//
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    data_handle__<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & h
  )
  {
  } // handle

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    sparse_data_handle__<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & h
  )
  {

  } // handle

  //--------------------------------------------------------------------------//
  //! The finalize_handles_t type can be called to walk task args after task
  //! execution. This allows us to free memory allocated during the task.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t PERMISSIONS
  >
  void
  handle(
    data_client_handle__<T, PERMISSIONS> & h
  )
  {
    h.delete_storage();
  } // handle

  template<
    typename T
  >
  void
  handle(
    mutator_handle__<
      T
    > & h
  )
  {
    using offset_t = typename mutator_handle__<T>::offset_t;
    using entry_value_t = typename mutator_handle__<T>::entry_value_t;
    using commit_info_t = typename mutator_handle__<T>::commit_info_t;

    if(*h.num_exclusive_insertions > *h.reserve){
      size_t old_exclusive_entries = *h.num_exclusive_entries;
      size_t old_reserve = *h.reserve;

      size_t needed = *h.num_exclusive_insertions - *h.reserve;

      *h.num_exclusive_entries += *h.num_exclusive_insertions;
      *h.reserve = std::max(h.reserve_chunk, needed);

      constexpr size_t entry_value_size = sizeof(size_t) + sizeof(T);

      size_t count = *h.num_exclusive_entries + *h.reserve + 
        (h.num_shared() + h.num_ghost()) * h.max_entries_per_index();

      std::vector<uint8_t> new_entries(count * entry_value_size);

      std::memcpy(&new_entries[0], &h.entries[0], 
                  old_exclusive_entries * entry_value_size);

      std::memcpy(&new_entries[0] + *h.num_exclusive_entries + *h.reserve, 
        &h.entries[0] + old_exclusive_entries + old_reserve,
        (h.num_shared() + h.num_ghost()) * h.max_entries_per_index() *
        entry_value_size);

      *h.entries = std::move(new_entries);

      size_t num_total = h.num_exclusive() + h.num_shared() + h.num_ghost();

      for(size_t i = h.num_exclusive(); i < num_total; ++i){
        offset_t& oi = (*h.offsets)[i];
        oi.set_offset(*h.num_exclusive_entries + *h.reserve + i * 
          h.max_entries_per_index());
      }
    }

    delete h.num_exclusive_insertions;

    entry_value_t* entries = 
      reinterpret_cast<entry_value_t*>(&(*h.entries)[0]);

    commit_info_t ci;
    ci.offsets = &(*h.offsets)[0];
    ci.entries[0] = entries;
    ci.entries[1] = entries + *h.num_exclusive_entries + *h.reserve;
    ci.entries[2] = ci.entries[1] + h.num_shared() * h.max_entries_per_index();

    h.commit(&ci);

  } // handle

  //-----------------------------------------------------------------------//
  // If this is not a data handle, then simply skip it.
  //-----------------------------------------------------------------------//

  template<
    typename T
  >
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base_t, T>::value>
  handle(
    T &
  )
  {
  } // handle

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_topology_finalize_handles_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
