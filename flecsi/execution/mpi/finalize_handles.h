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


#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/data/ragged_mutator.h>

namespace flecsi {
namespace execution {

struct finalize_handles_t : public utils::tuple_walker__<finalize_handles_t>
{
  /*!
  Nothing needs to be done to finalize a dense data handle.
   */
  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    dense_data_handle__<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & h
  )
  {
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

    auto &context = context_t::instance();
    const int my_color = context.color();
    auto &my_coloring_info =
      context.coloring_info(h.index_space).at(my_color);
    auto index_coloring = context.coloring(h.index_space);

    auto &sparse_field_metadata =
      context.registered_sparse_field_metadata().at(h.fid);

    entry_value_t *entries =
        reinterpret_cast<entry_value_t *>(&(*h.entries)[0]);
    auto offsets = &(*h.offsets)[0];
    auto shared_data = entries + *h.reserve;//ci.entries[1];
    auto ghost_data = shared_data + h.num_shared() * h.max_entries_per_index();///ci.entries[2];

    // Get entry_values
    MPI_Datatype shared_ghost_type;
    MPI_Type_contiguous(
      sizeof(entry_value_t),
      MPI_BYTE, &shared_ghost_type);
    MPI_Type_commit(&shared_ghost_type);

    MPI_Win win;
    MPI_Win_create(shared_data,
                   sizeof(entry_value_t) * h.num_shared() * h.max_entries_per_index(),
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
      MPI_Get(&ghost_data[i*h.max_entries_per_index()],
              h.max_entries_per_index(),
              shared_ghost_type,
              ghost.rank,
              ghost.offset*h.max_entries_per_index(),
              h.max_entries_per_index(),
              shared_ghost_type,
              win);
      i++;
    }

    MPI_Win_complete(win);
    MPI_Win_wait(win);

    MPI_Win_free(&win);

    for (int i = 0; i < h.num_ghost() * h.max_entries_per_index(); i++)
      clog_rank(warn, 0) << "ghost after: " << ghost_data[i].value << std::endl;

    int send_count = 0;
    for (auto& shared : index_coloring.shared) {
      send_count += shared.shared.size();
    }

    // Send/Recv counts in entry_values.
    std::vector<MPI_Request> requests(send_count);
    std::vector<MPI_Status> statuses(send_count);
    std::vector<MPI_Request> recv_requests(h.num_ghost());
    std::vector<MPI_Status> recv_status(h.num_ghost());

    std::vector<uint32_t> send_count_buf;
    for (auto& shared : index_coloring.shared) {
      for (auto peer : shared.shared) {
        send_count_buf.push_back(offsets[h.num_exclusive() + shared.offset].count());
      }
    }

    i = 0;
    for (auto& shared : index_coloring.shared) {
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
    for (auto& ghost : index_coloring.ghost) {
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

    for (int i = 0; i < h.num_ghost(); i++) {
      clog_rank(warn, 0) << recv_count_buf[i] << std::endl;
      offsets[h.num_exclusive() + h.num_shared() + i].set_count(recv_count_buf[i]);
    }
  } // handle
 
  /*!
    Finalize set topology storage. This inspects index based sizes and
    writes out appropriate metadata.  
   */
  template<
    typename T,
    size_t PERMISSIONS
  >
  typename std::enable_if_t<std::is_base_of<topology::set_topology_base_t, T>
    ::value>
  handle(
    data_client_handle__<T, PERMISSIONS> & h
  )
  {
    auto& context_ = context_t::instance();

    auto storage = h.storage();
    storage->finalize_storage();
  } // handle

  /*!
   No special handling is currently needed here. No-op.
   */
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

  /*!
   The finalize_handles_t type can be called to walk task args after task
   execution. This allows us to free memory allocated during the task.
  
   @ingroup execution
   */

  template<
    typename T,
    size_t PERMISSIONS
  >
  typename std::enable_if_t<std::is_base_of<topology::mesh_topology_base_t, T>
    ::value>
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
    handle(reinterpret_cast<sparse_mutator<T>&>(m));
  }

  //-----------------------------------------------------------------------//
  // If this is not a data handle, then simply skip it.
  //-----------------------------------------------------------------------//

  template<
    typename T
  >
  static
  typename std::enable_if_t<!std::is_base_of<dense_data_handle_base_t, T>::value &&
  !std::is_base_of<data_client_handle_base_t, T>::value>
  handle(
    T &
  )
  {
  } // handle

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi
