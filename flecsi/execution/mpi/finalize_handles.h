/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_finalize_handles_h
#define flecsi_execution_mpi_finalize_handles_h

#include "flecsi/data/dense_accessor.h"
#include "flecsi/data/sparse_accessor.h"
#include "flecsi/data/sparse_mutator.h"
#include "flecsi/data/ragged_mutator.h"

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
    sparse_accessor<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & a
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
    ragged_accessor<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & a
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
    sparse_mutator<
      T
    > & m
  )
  {
    auto& h = m.h_;

    using offset_t = typename mutator_handle__<T>::offset_t;
    using entry_value_t = typename mutator_handle__<T>::entry_value_t;
    using commit_info_t = typename mutator_handle__<T>::commit_info_t;

    size_t free_reserve = *h.reserve - *h.num_exclusive_entries;

    if(*h.num_exclusive_insertions > free_reserve){
      size_t old_exclusive_entries = *h.num_exclusive_entries;
      size_t old_reserve = *h.reserve;

      size_t needed = *h.num_exclusive_insertions - free_reserve;

      *h.num_exclusive_entries += *h.num_exclusive_insertions;
      *h.reserve += std::max(h.reserve_chunk, needed);

      constexpr size_t entry_value_size = sizeof(size_t) + sizeof(T);

      size_t n = h.num_shared() + h.num_ghost();

      size_t count = *h.reserve + n * h.max_entries_per_index();

      h.entries->resize(count * entry_value_size);

      entry_value_t* tmp = new entry_value_t[n * h.max_entries_per_index()];

      size_t bytes = n * h.max_entries_per_index() * entry_value_size;

      std::memcpy(tmp, &(*h.entries)[0] + old_reserve * 
        entry_value_size, bytes);

      std::memcpy(&(*h.entries)[0] + *h.reserve *
        entry_value_size, tmp, bytes);

      delete[] tmp;
      
      size_t ne = h.num_exclusive();

      for(size_t i = 0; i < n; ++i){
        offset_t& oi = (*h.offsets)[i + ne];
        oi.set_offset(*h.reserve + i * h.max_entries_per_index());
      }
    }

    delete h.num_exclusive_insertions;

    entry_value_t* entries = 
      reinterpret_cast<entry_value_t*>(&(*h.entries)[0]);

    commit_info_t ci;
    ci.offsets = &(*h.offsets)[0];
    ci.entries[0] = entries;
    ci.entries[1] = entries + *h.reserve;
    ci.entries[2] = ci.entries[1] + h.num_shared() * h.max_entries_per_index();

    h.commit(&ci);

  } // handle

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
    // TODO: fix
    handle(reinterpret_cast<sparse_mutator<T>&>(m));
  }

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
