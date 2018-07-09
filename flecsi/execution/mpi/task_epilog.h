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


#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/data/ragged_mutator.h>

/*!
 @file
 @date Initial file creation: May 19, 2017
 */

#include <vector>

#include "mpi.h"
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/context.h>
#include <flecsi/coloring/mpi_utils.h>

namespace flecsi {
namespace execution {

  /*!
   The task_epilog_t type can be called to walk the task args after the
   task has run. This allows synchronization dependencies to be added
   to the execution flow.
  
   @ingroup execution
   */

  struct task_epilog_t : public utils::tuple_walker__<task_epilog_t> {

    /*!
     Construct a task_epilog_t instance.
     */

    task_epilog_t() = default;

    /*!
     FIXME: Need a description.
    
     @tparam T                     The data type referenced by the handle.
     @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                                   indices of the index partition.
     @tparam SHARED_PERMISSIONS    The permissions required on the shared
                                   indices of the index partition.
     @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                                   indices of the index partition.
    
     */

    template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS
    >
    void
    handle(
     dense_accessor<
       T,
       EXCLUSIVE_PERMISSIONS,
       SHARED_PERMISSIONS,
       GHOST_PERMISSIONS
     > & a
    )
    {
      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

      auto& h = a.handle;

      auto &context = context_t::instance();
      const int my_color = context.color();
      auto &my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);

      auto &field_metadata = context.registered_field_metadata().at(h.fid);

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
      size_t PERMISSIONS
    >
    void
    handle(
     global_accessor__<
       T,
       PERMISSIONS
     > & a
    )
    {
      auto& h = a.handle;

      // Skip Read Only handles
       if (PERMISSIONS == ro)
         return;

        auto &context = context_t::instance();
        const int my_color = context.color();
        MPI_Bcast(&a.data(), 1, flecsi::coloring::mpi_typetraits__<T>::type(), 0,
        MPI_COMM_WORLD); 
    } // handle


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
    ) {
      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

      auto &h = a.handle;

      auto &context = context_t::instance();
      const int my_color = context.color();
      auto &my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);
      auto index_coloring = context.coloring(h.index_space);

      auto &sparse_field_metadata =
        context.registered_sparse_field_metadata().at(h.fid);

      using entry_value_t = typename mutator_handle__<T>::entry_value_t;

      entry_value_t *entries = h.entries;
      auto offsets = &(h.offsets)[0];
      auto shared_data = entries + h.exclusive_reserve;
      auto ghost_data = shared_data + h.num_shared_ * h.max_entries_per_index;

      MPI_Datatype shared_ghost_type = sparse_field_metadata.shared_ghost_type;
    
      MPI_Win win = sparse_field_metadata.win;

      MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
      MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);
  
      // TODO: consolidate MPI_Get using the fancy MPI_Datatype trick.
      int i = 0;
      for (auto& ghost : index_coloring.ghost) {
        clog_rank(warn, 0) << "ghost id: " << ghost.id << ", rank: "
                           << ghost.rank
                           << ", offset: " << ghost.offset
                           << std::endl;
        MPI_Get(&ghost_data[i*h.max_entries_per_index],
                h.max_entries_per_index,
                shared_ghost_type,
                ghost.rank,
                ghost.offset*h.max_entries_per_index,
                h.max_entries_per_index,
                shared_ghost_type,
                win);
        i++;
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);
    } // handle

    template<
      typename T
    >
    void
    handle(
      sparse_mutator<
      T
      > &m
    ) {
      auto &h = m.h_;

      using offset_t = typename mutator_handle__<T>::offset_t;
      using entry_value_t = typename mutator_handle__<T>::entry_value_t;
      using commit_info_t = typename mutator_handle__<T>::commit_info_t;

      clog_assert(*h.num_exclusive_insertions <= h.exclusive_reserve,
                  "sparse exclusive reserve exceed");

      delete h.num_exclusive_insertions;

      entry_value_t *entries =
        reinterpret_cast<entry_value_t *>(&(*h.entries)[0]);

      commit_info_t ci;
      ci.offsets = &(*h.offsets)[0];
      ci.entries[0] = entries;
      ci.entries[1] = entries + h.exclusive_reserve;
      ci.entries[2] =
        ci.entries[1] + h.num_shared() * h.max_entries_per_index();

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
      handle(reinterpret_cast<sparse_mutator<T>&>(m));
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

    /*!
      This method is called on any task arguments that are not handles, e.g.
      scalars or those that did not need any special handling.
     */
    template<
      typename T
    >
    static
    typename
    std::enable_if_t<!std::is_base_of<dense_accessor_base_t, T>::value>
    handle(
      T&
    )
    {
    } // handle

  }; // struct task_epilog_t

} // namespace execution
} // namespace flecsi
