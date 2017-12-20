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
      auto& h = a.handle;

      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

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
      auto &h = a.handle;

      using offset_t = typename mutator_handle__<T>::offset_t;
      using entry_value_t = typename mutator_handle__<T>::entry_value_t;
      using commit_info_t = typename mutator_handle__<T>::commit_info_t;

      // Skip Read Only handles
      if (EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
        return;

      auto &context = context_t::instance();
      const int my_color = context.color();
      auto &my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);
      auto index_coloring = context.coloring(h.index_space);

      auto &sparse_field_metadata =
        context.registered_sparse_field_metadata().at(h.fid);

      entry_value_t *entries = h.entries;
      auto offsets = &(h.offsets)[0];
      auto shared_data = entries + h.reserve;
      auto ghost_data = shared_data + h.num_shared_ * h.max_entries_per_index;

      // Get entry_values
      MPI_Datatype shared_ghost_type;
      MPI_Type_contiguous(
        sizeof(entry_value_t),
        MPI_BYTE, &shared_ghost_type);
      MPI_Type_commit(&shared_ghost_type);

      MPI_Win win;
      MPI_Win_create(shared_data,
                     sizeof(entry_value_t) * h.num_shared_ * h.max_entries_per_index,
                     sizeof(entry_value_t),
                     MPI_INFO_NULL, MPI_COMM_WORLD,
                     &win);

      MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
      MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);

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

      MPI_Win_free(&win);

      for (int i = 0; i < h.num_ghost_ * h.max_entries_per_index; i++)
        clog_rank(warn, 0) << "ghost after: " << ghost_data[i].value << std::endl;

      int send_count = 0;
      for (auto& shared : index_coloring.shared) {
        send_count += shared.shared.size();
      }

      // Send/Recv counts in entry_values.
      std::vector<MPI_Request> requests(send_count + h.num_ghost_);
      std::vector<MPI_Status> statuses(send_count + h.num_ghost_);

      std::vector<uint32_t> send_count_buf;
      for (auto& shared : index_coloring.shared) {
        for (auto peer : shared.shared) {
          send_count_buf.push_back(offsets[h.num_exclusive_ + shared.offset].count());
        }
      }

      i = 0;
      for (auto& shared : index_coloring.shared) {
        for (auto peer : shared.shared) {
          MPI_Isend(&send_count_buf[i],
                    1,
                    flecsi::coloring::mpi_typetraits__<uint32_t>::type(),
                    peer, shared.id, MPI_COMM_WORLD, &requests[i]);
          i++;
        }
      }

      std::vector<uint32_t> recv_count_buf(h.num_ghost_);
      i = 0;
      for (auto& ghost : index_coloring.ghost) {
        MPI_Status status;
        MPI_Irecv(&recv_count_buf[i],
                  1,
                  flecsi::coloring::mpi_typetraits__<uint32_t>::type(),
                  ghost.rank, ghost.id, MPI_COMM_WORLD, &requests[i+send_count]);
        i++;
      }

      MPI_Waitall(send_count + h.num_ghost_,
                  requests.data(),
                  statuses.data());

      for (int i = 0; i < h.num_ghost_; i++) {
        clog_rank(warn, 0) << recv_count_buf[i] << std::endl;
        offsets[h.num_exclusive_ + h.num_shared_ + i].set_count(recv_count_buf[i]);
      }
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

      if (*h.num_exclusive_insertions > *h.reserve) {
        size_t old_exclusive_entries = *h.num_exclusive_entries;
        size_t old_reserve = *h.reserve;

        size_t needed = *h.num_exclusive_insertions - *h.reserve;

        *h.num_exclusive_entries += *h.num_exclusive_insertions;
        *h.reserve += std::max(h.reserve_chunk, needed);

        constexpr size_t entry_value_size = sizeof(size_t) + sizeof(T);

        size_t count = *h.reserve +
                       (h.num_shared() + h.num_ghost()) *
                       h.max_entries_per_index();

        h.entries->resize(count * entry_value_size);

        entry_value_t *tmp =
          new entry_value_t[(h.num_shared() + h.num_ghost()) *
                            h.max_entries_per_index()];

        size_t bytes =
          (h.num_shared() + h.num_ghost()) * h.max_entries_per_index() *
          entry_value_size;

        std::memcpy(tmp, &(*h.entries)[0] + old_reserve *
                                            entry_value_size, bytes);

        std::memcpy(&(*h.entries)[0] + *h.reserve *
                                       entry_value_size, tmp, bytes);

        delete[] tmp;

        size_t n = h.num_shared() + h.num_ghost();
        size_t ne = h.num_exclusive();

        for (size_t i = 0; i < n; ++i) {
          offset_t &oi = (*h.offsets)[i + ne];
          oi.set_offset(*h.reserve + i * h.max_entries_per_index());
        }
      }

      delete h.num_exclusive_insertions;

      entry_value_t *entries =
        reinterpret_cast<entry_value_t *>(&(*h.entries)[0]);

      commit_info_t ci;
      ci.offsets = &(*h.offsets)[0];
      ci.entries[0] = entries;
      ci.entries[1] = entries + *h.reserve;
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
