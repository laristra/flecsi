/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_finalize_handles_h
#define flecsi_execution_mpi_finalize_handles_h

#include "flecsi/data/dense_accessor.h"
#include "flecsi/data/sparse_accessor.h"
#include "flecsi/data/mutator.h"
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

//  template<
//    typename T,
//    size_t EXCLUSIVE_PERMISSIONS,
//    size_t SHARED_PERMISSIONS,
//    size_t GHOST_PERMISSIONS
//  >
//  void
//  handle(
//    sparse_accessor<
//      T,
//      EXCLUSIVE_PERMISSIONS,
//      SHARED_PERMISSIONS,
//      GHOST_PERMISSIONS
//    > & a
//  )
//  {
//
//  } // handle
  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
//    sparse_accessor<
//    T,
//    EXCLUSIVE_PERMISSIONS,
//    SHARED_PERMISSIONS,
//    GHOST_PERMISSIONS
//    > & a
    mutator<
    T
    > & m
  )
  {
    auto& h = m.h_;

    // Skip Read Only handles
    if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
      return;

    auto& context = context_t::instance();
    const int my_color = context.color();
    auto& my_coloring_info =
      context.coloring_info(h.index_space).at(my_color);

    auto& sparse_field_metadata =
      context.registered_sparse_field_metadata().at(h.fid);

    MPI_Win win = sparse_field_metadata.win;

    MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
    MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);

    for (auto ghost_owner : my_coloring_info.ghost_owners) {
      MPI_Get(h.entries + (h.reserve + h.num_shared_) *  (sizeof(size_t) +
                sizeof(T)), 1,
              sparse_field_metadata.origin_types[ghost_owner],
              ghost_owner, 0, 1,
              sparse_field_metadata.target_types[ghost_owner],
              win);
    }

    MPI_Win_complete(win);
    MPI_Win_wait(win);

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
    ragged_mutator<
      T
    > & m
  )
  {
    // TODO: fix
    handle(reinterpret_cast<mutator<T>&>(m));
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
