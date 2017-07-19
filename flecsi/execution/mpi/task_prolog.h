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

#ifndef flecsi_execution_mpi_task_prolog_h
#define flecsi_execution_mpi_task_prolog_h

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
  //! The task_prolog_t type can be called to walk the task args after the
  //! task launcher is created, but before the task has run. This allows
  //! synchronization dependencies to be added to the execution flow.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct task_prolog_t : public utils::tuple_walker__<task_prolog_t>
  {

    //------------------------------------------------------------------------//
    //! Construct a task_prolog_t instance.
    //!
    //------------------------------------------------------------------------//

    task_prolog_t() = default;


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
      auto& flecsi_context = context_t::instance();

      const int my_color = flecsi_context.rank;
      auto &coloring_info = flecsi_context.coloring_info(h.index_space);

      auto &my_coloring_info = flecsi_context.coloring_info(h.index_space).at(my_color);

      std::vector<int> peers;
      std::set_union(my_coloring_info.shared_users.begin(), my_coloring_info.shared_users.end(),
                     my_coloring_info.ghost_owners.begin(), my_coloring_info.ghost_owners.end(),
                     std::back_inserter(peers));

      MPI_Group comm_grp, rma_group;
      MPI_Comm_group(MPI_COMM_WORLD, &comm_grp);
      MPI_Group_incl(comm_grp, peers.size(), peers.data(), &rma_group);
      MPI_Group_free(&comm_grp);

      // A pull model using MPI_Get:
      // 1. create MPI window for shared portion of the local buffer.
      MPI_Win win;
      MPI_Win_create(h.shared_data, my_coloring_info.shared * sizeof(T),
                     sizeof(T), MPI_INFO_NULL, MPI_COMM_WORLD,
                     &win);

      // 2. iterate through each ghost cell and MPI_Get from the peer.
      // FIXME: the group for MPI_Win_post are the "origin" processes, i.e. the peer
      // processes calling MPI_Get to get our shared cells. Thus granting access of
      // local window to these processes. This is the set union of the entry_info.shared
      // of shared cells.
      // On the other hand, the group for MPI_Win_start are the 'target' processes, i.e. the
      // peer processes this rank is going to get ghost cells from. This is the union of
      // entry_info.rank of ghost cells.
      MPI_Win_post(rma_group, 0, win);
      MPI_Win_start(rma_group, 0, win);


      std::vector<T> buffer(my_coloring_info.ghost);
      auto index_coloring = flecsi_context.coloring(h.index_space);

      int i = 0;
      for (auto ghost : index_coloring.ghost) {
        clog_rank(warn, 1) << "ghost id: " <<  ghost.id << ", rank: " << ghost.rank
                            << ", offset: " << ghost.offset
                            << std::endl;
        MPI_Get(h.ghost_data+i, 1, MPI_UNSIGNED_LONG_LONG,
                ghost.rank, ghost.offset,
                1, flecsi::coloring::mpi_typetraits<T>::type(), win);
        i++;
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);

      MPI_Group_free(&rma_group);
      MPI_Win_free(&win);


      for (int i = 0; i < h.ghost_size; i++) {
        clog_rank(warn, 1) << "ghost data: " << h.ghost_data[i] << std::endl;
      }

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

  }; // struct task_prolog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_task_prolog_h
