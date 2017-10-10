/*~--------------------------------------------------------------------------~*
*  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
* /@@/////  /@@          @@////@@ @@////// /@@
* /@@       /@@  @@@@@  @@    // /@@       /@@
* /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
* /@@////   /@@/@@@@@@@/@@       ////////@@/@@
* /@@       /@@/@@//// //@@    @@       /@@/@@
* /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
* //       ///  //////   //////  ////////  //
*
* Copyright (c) 2016 Los Alamos National Laboratory, LLC
* All rights reserved
*~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_task_epilog_h
#define flecsi_execution_mpi_task_epilog_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

#include <vector>

#include "mpi.h"
#include "flecsi/data/data.h"
#include "flecsi/execution/context.h"
#include "flecsi/coloring/mpi_utils.h"

namespace flecsi {
namespace execution {

  //--------------------------------------------------------------------------//
  //! The task_epilog_t type can be called to walk the task args after the
  //! task has run. This allows synchronization dependencies to be added
  //! to the execution flow.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct task_epilog_t : public utils::tuple_walker__<task_epilog_t>
  {

    //------------------------------------------------------------------------//
    //! Construct a task_epilog_t instance.
    //!
    //------------------------------------------------------------------------//

    task_epilog_t() = default;

    //------------------------------------------------------------------------//
    //! FIXME: Need a description.
    //!
    //! @tparam T                     The data type referenced by the handle.
    //! @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
    //!                               indices of the index partition.
    //! @tparam SHARED_PERMISSIONS    The permissions required on the shared
    //!                               indices of the index partition.
    //! @tparam GHOST_PERMISSIONS     The permissions required on the ghost
    //!                               indices of the index partition.
    //!
    //------------------------------------------------------------------------//

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
      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

      auto& context = context_t::instance();
      const int my_color = context.color();
      auto& my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);

      auto& field_metadata = context.registered_field_metadata().at(h.fid);

      MPI_Win win = field_metadata.win;

      MPI_Win_post(field_metadata.shared_users_grp, 0, win);
      MPI_Win_start(field_metadata.ghost_owners_grp, 0, win);

      for (auto ghost_owner : my_coloring_info.ghost_owners) {
        MPI_Get(h.ghost_data, 1, field_metadata.origin_types[ghost_owner],
                ghost_owner, 0, 1, field_metadata.target_types[ghost_owner],
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
      sparse_data_handle__<
        T,
        EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS,
        GHOST_PERMISSIONS
      > & h
    )
    {
      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

      auto& context = context_t::instance();
      const int my_color = context.color();
      auto& my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);

      auto& sparse_field_metadata = 
        context.registered_sparse_field_metadata().at(h.fid);

#if 0
      MPI_Win win = sparse_field_metadata.win;

      MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
      MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);

      for (auto ghost_owner : my_coloring_info.ghost_owners) {
        MPI_Get(h.ghost_entries, 1,
                sparse_field_metadata.origin_types[ghost_owner],
                ghost_owner, 0, 1,
                sparse_field_metadata.target_types[ghost_owner],
                win);
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);
#endif
    } // handle

    //------------------------------------------------------------------------//
    //! FIXME: Need to document.
    //------------------------------------------------------------------------//

    template<
      typename T
    >
    static
    typename std::enable_if_t<!std::is_base_of<data_handle_base_t, T>::value>
    handle(
      T&
    )
    {
    } // handle

  }; // struct task_epilog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_task_epilog_h
