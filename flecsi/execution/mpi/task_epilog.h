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
  //! task launcher is created, but before the task has run. This allows
  //! synchronization dependencies to be added to the execution flow.
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
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
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
      // TODO: skip internal data handle?
      // TODO: extract window/group init code?
      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

      auto& context = context_t::instance();
      const int my_color = context.color();
      auto &my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);

      // The group for MPI_Win_post are the "origin" processes, i.e.
      // the peer processes calling MPI_Get to get our shared cells. Thus
      // granting access of local window to these processes. This is the set
      // coloring_info_t::shared_users
      // On the other hand, the group for MPI_Win_start are the 'target'
      // processes, i.e. the peer processes this rank is going to get ghost
      // cells from. This is the set coloring_info_t::ghost_owners.
      // Since both shared_users and ghost_owners are std::set, we have copy
      // them to std::vector be passed to MPI.
      std::vector<int> shared_users(my_coloring_info.shared_users.begin(),
                                    my_coloring_info.shared_users.end());
      std::vector<int> ghost_owners(my_coloring_info.ghost_owners.begin(),
                                    my_coloring_info.ghost_owners.end());

      MPI_Group comm_grp, shared_users_grp, ghost_owners_grp;
      MPI_Comm_group(MPI_COMM_WORLD, &comm_grp);
      MPI_Group_incl(comm_grp, shared_users.size(),
                     shared_users.data(), &shared_users_grp);
      MPI_Group_incl(comm_grp, ghost_owners.size(),
                     ghost_owners.data(), &ghost_owners_grp);

      // A pull model using MPI_Get:
      // 1. create MPI window for shared portion of the local buffer.
      MPI_Win win;
      MPI_Win_create(h.shared_data, my_coloring_info.shared * sizeof(T),
                     sizeof(T), MPI_INFO_NULL, MPI_COMM_WORLD,
                     &win);

      // 2. iterate through each ghost cell and MPI_Get from the peer.
      MPI_Win_post(shared_users_grp, 0, win);
      MPI_Win_start(ghost_owners_grp, 0, win);

      auto index_coloring = context.coloring(h.index_space);

      int i = 0;
      for (auto ghost : index_coloring.ghost) {
//        clog_rank(warn, 1) << "ghost id: " <<  ghost.id << ", rank: " << ghost.rank
//                            << ", offset: " << ghost.offset
//                            << std::endl;
        MPI_Get(h.ghost_data+i, 1,
                flecsi::coloring::mpi_typetraits__<T>::type(),
                ghost.rank, ghost.offset, 1,
                flecsi::coloring::mpi_typetraits__<T>::type(), win);
        i++;
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);
    
      MPI_Win_free(&win);

      MPI_Group_free(&shared_users_grp);
      MPI_Group_free(&ghost_owners_grp);
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
