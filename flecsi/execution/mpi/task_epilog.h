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

#include "flecsi/data/dense_accessor.h"
#include "flecsi/data/sparse_accessor.h"
#include "flecsi/data/ragged_accessor.h"
#include "flecsi/data/mutator.h"
#include "flecsi/data/ragged_mutator.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

#include <vector>

#include "mpi.h"
#include "flecsi/data/data.h"
#include "flecsi/data/dense_accessor.h"
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

  struct task_epilog_t : public utils::tuple_walker__<task_epilog_t> {

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
      dense_accessor <
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
      > &a
    ) {
      auto &h = a.handle;

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
      typename T
    >
    void
    handle(
      mutator <
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

      ci.offsets = &(*h.offsets)[0];
      ci.entries[0] = entries;
      ci.entries[1] = entries + *h.reserve;
      ci.entries[2] =
        ci.entries[1] + h.num_shared() * h.max_entries_per_index();

      auto &context = context_t::instance();
      const int my_color = context.color();
      auto &my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);
      auto index_coloring = context.coloring(h.index_space);

      auto &sparse_field_metadata =
        context.registered_sparse_field_metadata().at(h.fid);

      MPI_Datatype shared_ghost_type;
      MPI_Type_contiguous(
        sizeof(entry_value_t),
        MPI_BYTE, &shared_ghost_type);
      MPI_Type_commit(&shared_ghost_type);

      auto shared_data = ci.entries[1];
      auto ghost_data = ci.entries[2];

#if 1
      int send_count = 0;
      for (auto& shared : index_coloring.shared) {
        send_count += shared.shared.size();
      }

      std::vector<MPI_Request> requests(send_count);
      std::vector<MPI_Status> statuses(send_count);

      int i = 0;

      for (auto& shared : index_coloring.shared) {
        for (auto peer : shared.shared) {
          clog_rank(warn, 0) << "shared id: " << shared.id << ", rank: "
                             << shared.rank
                             << ", offset: " << shared.offset
                             << std::endl;
          MPI_Isend(&shared_data[(shared.offset)*h.max_entries_per_index()],
                    h.max_entries_per_index(),
                    shared_ghost_type,
                    peer, shared.id, MPI_COMM_WORLD, &requests[i++]);
        }
      }

      clog_rank(warn, 0) << "send_count: " << send_count << std::endl;

      std::vector<MPI_Request> recv_requests(h.num_ghost());
      std::vector<MPI_Status> recv_status(h.num_ghost());
      i = 0;
      for (auto& ghost : index_coloring.ghost) {
        clog_rank(warn, 0) << "ghost id: " << ghost.id << ", rank: "
                           << ghost.rank
                           << ", offset: " << ghost.offset
                           << std::endl;
        MPI_Irecv(&ghost_data[i*h.max_entries_per_index()],
                  h.max_entries_per_index(),
                  shared_ghost_type,
                  ghost.rank, ghost.id, MPI_COMM_WORLD, &recv_requests[i]);
        i++;
      }

      MPI_Waitall(send_count,
                  requests.data(),
                  statuses.data());
      MPI_Waitall(h.num_ghost(),
                  recv_requests.data(),
                  recv_status.data());

      for (int i = 0; i < h.num_ghost() * h.max_entries_per_index(); i++)
          clog_rank(warn, 0) << "ghost after: " << ghost_data[i].value << std::endl;

      std::vector<uint32_t> send_count_buf;
      for (auto shared : index_coloring.shared) {
        for (auto peer : shared.shared) {
          send_count_buf.push_back(ci.offsets[h.num_exclusive() + shared.offset].count());
        }
      }

      i = 0;
      for (auto shared : index_coloring.shared) {
        for (auto peer : shared.shared) {
          MPI_Isend(&send_count_buf[i],
                    1,
                    flecsi::coloring::mpi_typetraits__<uint32_t>::type(),
                    peer, 99, MPI_COMM_WORLD, &requests[i]);
          i++;
        }
      }

      std::vector<uint32_t> recv_count_buf(h.num_ghost());
      i = 0;
      for (auto ghost : index_coloring.ghost) {
        MPI_Status status;
        MPI_Irecv(&recv_count_buf[i],
                  1,
                  flecsi::coloring::mpi_typetraits__<uint32_t>::type(),
                  ghost.rank, 99, MPI_COMM_WORLD, &recv_requests[i]);
        i++;
      }

      MPI_Waitall(send_count,
                  requests.data(),
                  statuses.data());
      MPI_Waitall(h.num_ghost(),
                  recv_requests.data(),
                  recv_status.data());

      //for (auto& ghost : index_coloring.ghost) {
      for (int i = 0; i < h.num_ghost(); i++) {
        if (my_color == 0) {
          std::cout << recv_count_buf[i] << " ";
        }

        clog_rank(warn, 0) << recv_count_buf[i] << std::endl;
        ci.offsets[h.num_exclusive() + h.num_shared() + i].set_count(recv_count_buf[i]);
      }
#endif

#if 0
      MPI_Status status;
      if (my_color == 0)
        MPI_Send(&shared_data[2*h.max_entries_per_index()], h.max_entries_per_index(),
                 shared_ghost_type, 1, 99, MPI_COMM_WORLD);
      else {
        for (int i = 0; i < h.num_ghost()*h.max_entries_per_index(); i++)
          std::cout << "ghost before: " << ghost_data[i].value << std::endl;
        MPI_Recv(&ghost_data[2*h.max_entries_per_index()], h.max_entries_per_index(),
                 shared_ghost_type, 0, 99, MPI_COMM_WORLD,
                 &status);
        for (int i = 0; i < h.num_ghost() * h.max_entries_per_index(); i++)
          std::cout << "ghost after: " << ghost_data[i].value << std::endl;
      }

      MPI_Datatype offset_type;
      MPI_Type_contiguous(sizeof(data::sparse_data_offset_t),
                          MPI_BYTE, &offset_type);
      MPI_Type_commit(&offset_type);

      uint32_t count_buf;
      if (my_color == 0) {
        count_buf = ci.offsets[h.num_exclusive()].count();
        MPI_Send(&count_buf, 1, offset_type, 1, 99,
                 MPI_COMM_WORLD);
      } else {
        for (int i = 0; i < 40; i++)
          std::cout << "ghost offset before: " << i << "," <<  ci.offsets[i].count() << std::endl;
        // TODO: we can not simply copy offset as is, it will destroy the offset
        // part while we only want to copy the count part.
        MPI_Recv(&count_buf,
                 1, offset_type, 0, 99, MPI_COMM_WORLD,
                 &status);
        ci.offsets[h.num_exclusive() + h.num_shared()].set_count(count_buf);
        for (int i = 0; i < 40; i++)
          std::cout << "ghost offset after: " << i << "," <<  ci.offsets[i].count() << std::endl;
      }
#endif

//      MPI_Win win = sparse_field_metadata.win;
//
//      MPI_Win_attach(sparse_field_metadata.win, ci.entries[1],
//                     h.num_shared() * h.max_entries_per_index() *
//                     sizeof(entry_value_t));
//
//      MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
//      MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);




//      for (int i = 0; i < h.num_ghost() * h.max_entries_per_index(); i++) {
//        std::cout << ghost_data[i].value << " ";
//      }
//      std::cout << std::endl;


//      int i = 0;
//      for (auto ghost : index_coloring.ghost) {
//        clog_rank(warn, 1) << "ghost id: " <<  ghost.id << ", rank: " << ghost.rank
//                           << ", offset: " << ghost.offset
//                           << std::endl;
//        MPI_Get(&ci.entries[2][i*h.max_entries_per_index()], h.max_entries_per_index(),
//                shared_ghost_type,
//                ghost.rank, ghost.offset * h.max_entries_per_index(), h.max_entries_per_index(),
//                shared_ghost_type, win);
//        i++;
//      }

//      for (auto ghost_owner : my_coloring_info.ghost_owners) {
//        MPI_Get(ci.entries[2],
//                1,
//                sparse_field_metadata.origin_types[ghost_owner],
//                ghost_owner, 0, 1,
//                sparse_field_metadata.target_types[ghost_owner],
//                win);
//      }

//      MPI_Win_complete(win);
//      MPI_Win_wait(win);

//      for (auto shared_user : my_coloring_info.shared_users) {
//        MPI_Isend()
//      }

//      auto ghost_data = ci.entries[2];
//      for (int i = 0; i < h.num_ghost() * h.max_entries_per_index(); i++) {
//        std::cout << ghost_data[i].value << " ";
//      }
//      std::cout << std::endl;

    } // handle

//    template<
//      typename T
//    >
//    void
//    handle(
//      mutator<
//      T
//      > & m
//    )
//    {
//      auto& h = m.h_;
//
//      using offset_t = typename mutator_handle__<T>::offset_t;
//      using entry_value_t = typename mutator_handle__<T>::entry_value_t;
//      using commit_info_t = typename mutator_handle__<T>::commit_info_t;
//
//      if(*h.num_exclusive_insertions > *h.reserve){
//        size_t old_exclusive_entries = *h.num_exclusive_entries;
//        size_t old_reserve = *h.reserve;
//
//        size_t needed = *h.num_exclusive_insertions - *h.reserve;
//
//        *h.num_exclusive_entries += *h.num_exclusive_insertions;
//        *h.reserve = std::max(h.reserve_chunk, needed);
//
//        constexpr size_t entry_value_size = sizeof(size_t) + sizeof(T);
//
//        size_t count = *h.num_exclusive_entries + *h.reserve +
//                       (h.num_shared() + h.num_ghost()) * h.max_entries_per_index();
//
//        h.entries->resize(count * entry_value_size);
//
//        entry_value_t* tmp =
//          new entry_value_t[(h.num_shared() + h.num_ghost()) *
//                            h.max_entries_per_index()];
//
//        size_t bytes =
//          (h.num_shared() + h.num_ghost()) * h.max_entries_per_index() *
//          entry_value_size;
//
//        std::memcpy(tmp, &(*h.entries)[0] +
//                         (old_exclusive_entries + old_reserve) * entry_value_size, bytes);
//
//        std::memcpy(&(*h.entries)[0] + (*h.num_exclusive_entries + *h.reserve) *
//                                       entry_value_size, tmp, bytes);
//
//        delete[] tmp;
//
//        size_t num_total = h.num_exclusive() + h.num_shared() + h.num_ghost();
//
//        for(size_t i = h.num_exclusive(); i < num_total; ++i){
//          offset_t& oi = (*h.offsets)[i];
//          oi.set_offset(*h.num_exclusive_entries + *h.reserve + i *
//                                                                h.max_entries_per_index());
//        }
//      }
//
//      delete h.num_exclusive_insertions;
//
//      entry_value_t* entries =
//        reinterpret_cast<entry_value_t*>(&(*h.entries)[0]);
//
//      commit_info_t ci;
//      ci.offsets = &(*h.offsets)[0];
//      ci.entries[0] = entries;
//      ci.entries[1] = entries + *h.num_exclusive_entries + *h.reserve;
//      ci.entries[2] = ci.entries[1] + h.num_shared() * h.max_entries_per_index();
//
//      h.commit(&ci);
//
//    } // handle


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
      auto& h = a.handle;

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

#endif // flecsi_execution_mpi_task_epilog_h
