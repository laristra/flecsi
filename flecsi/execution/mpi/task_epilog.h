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

#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>

/*!
 @file
 @date Initial file creation: May 19, 2017
 */

#include <cstring>
#include <stdint.h>
#include <vector>

#include "mpi.h"

#include <flecsi/coloring/mpi_utils.h>
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/context.h>

#include <flecsi/utils/tuple_walker.h>

namespace flecsi {
namespace execution {

/*!
 The task_epilog_t type can be called to walk the task args after the
 task has run. This allows synchronization dependencies to be added
 to the execution flow.

 @ingroup execution
 */

struct task_epilog_t : public flecsi::utils::tuple_walker_u<task_epilog_t> {

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

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(dense_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    // Skip Read Only handles
    if(EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
      return;

    auto & context = context_t::instance();
    const int my_color = context.color();
    auto & my_coloring_info = context.coloring_info(h.index_space).at(my_color);

    auto & field_metadata = context.registered_field_metadata().at(h.fid);

    MPI_Win win = field_metadata.win;

    MPI_Win_post(field_metadata.shared_users_grp, 0, win);
    MPI_Win_start(field_metadata.ghost_owners_grp, 0, win);

    for(auto ghost_owner : my_coloring_info.ghost_owners) {
      MPI_Get(h.ghost_data, 1, field_metadata.origin_types[ghost_owner],
        ghost_owner, 0, 1, field_metadata.target_types[ghost_owner], win);
    }

    MPI_Win_complete(win);
    MPI_Win_wait(win);
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor_u<T, PERMISSIONS> & a) {
    auto & h = a.handle;

    // Skip Read Only handles
    if(PERMISSIONS == ro)
      return;

    auto & context = context_t::instance();
    const int my_color = context.color();
    MPI_Bcast(&a.data(), 1, flecsi::coloring::mpi_typetraits_u<T>::type(), 0,
      MPI_COMM_WORLD);
  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(ragged_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    using value_t = T;

    // Skip Read Only handles
    if(EXCLUSIVE_PERMISSIONS == ro && SHARED_PERMISSIONS == ro)
      return;

    auto & context = context_t::instance();
    const int my_color = context.color();
    auto & my_coloring_info = context.coloring_info(h.index_space).at(my_color);
    auto index_coloring = context.coloring(h.index_space);

    auto & sparse_field_metadata =
      context.registered_sparse_field_metadata().at(h.fid);

    value_t * shared_data =
      new value_t[h.num_shared_ * h.max_entries_per_index];
    value_t * ghost_data = new value_t[h.num_ghost_ * h.max_entries_per_index];

    // Load data into shared data buffer
    for(int i = 0; i < h.num_shared_; ++i) {
      int r = i + h.num_exclusive_;
      const auto & row = h.new_entries[r];
      size_t count = row.size();
      std::memcpy(&shared_data[i * h.max_entries_per_index], row.begin(),
        count * sizeof(value_t));
    }

    // Get entry_values
    MPI_Datatype shared_ghost_type;
    MPI_Type_contiguous(sizeof(value_t), MPI_BYTE, &shared_ghost_type);
    MPI_Type_commit(&shared_ghost_type);

    MPI_Win win;
    MPI_Win_create(shared_data,
      sizeof(value_t) * h.num_shared_ * h.max_entries_per_index,
      sizeof(value_t), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
    MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);

    int i = 0;
    for(auto & ghost : index_coloring.ghost) {
      clog_rank(warn, 0) << "ghost id: " << ghost.id << ", rank: " << ghost.rank
                         << ", offset: " << ghost.offset << std::endl;
      MPI_Get(&ghost_data[i * h.max_entries_per_index], h.max_entries_per_index,
        shared_ghost_type, ghost.rank, ghost.offset * h.max_entries_per_index,
        h.max_entries_per_index, shared_ghost_type, win);
      i++;
    }

    MPI_Win_complete(win);
    MPI_Win_wait(win);

    MPI_Win_free(&win);

    for(int i = 0; i < h.num_ghost_ * h.max_entries_per_index; i++)
      clog_rank(warn, 0) << "ghost after: " << ghost_data[i] << std::endl;

    int send_count = 0;
    for(auto & shared : index_coloring.shared) {
      send_count += shared.shared.size();
    }

    // Send/Recv counts in entry_values.
    std::vector<MPI_Request> requests(send_count + h.num_ghost_);
    std::vector<MPI_Status> statuses(send_count + h.num_ghost_);

    std::vector<uint32_t> send_count_buf;
    for(auto & shared : index_coloring.shared) {
      for(auto peer : shared.shared) {
        send_count_buf.push_back(
          h.new_entries[h.num_exclusive_ + shared.offset].size());
      }
    }

    i = 0;
    for(auto & shared : index_coloring.shared) {
      for(auto peer : shared.shared) {
        MPI_Isend(&send_count_buf[i], 1,
          flecsi::coloring::mpi_typetraits_u<uint32_t>::type(), peer, shared.id,
          MPI_COMM_WORLD, &requests[i]);
        i++;
      }
    }

    std::vector<uint32_t> recv_count_buf(h.num_ghost_);
    i = 0;
    for(auto & ghost : index_coloring.ghost) {
      MPI_Status status;
      MPI_Irecv(&recv_count_buf[i], 1,
        flecsi::coloring::mpi_typetraits_u<uint32_t>::type(), ghost.rank,
        ghost.id, MPI_COMM_WORLD, &requests[i + send_count]);
      i++;
    }

    MPI_Waitall(send_count + h.num_ghost_, requests.data(), statuses.data());

    for(int i = 0; i < h.num_ghost_; i++) {
      clog_rank(warn, 0) << recv_count_buf[i] << std::endl;
    }

    // Unload data from ghost data buffer
    for(int i = 0; i < h.num_ghost_; i++) {
      int r = h.num_exclusive_ + h.num_shared_ + i;
      auto & row = h.new_entries[r];
      int count = recv_count_buf[i];
      row.resize(count);
      std::memcpy(row.begin(), &ghost_data[i * h.max_entries_per_index],
        count * sizeof(value_t));
    }

    delete[] shared_data;
    delete[] ghost_data;

  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(sparse_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    handle(a.ragged);
  } // handle

  template<typename T>
  void handle(ragged_mutator<T> & m) {} // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }

  /*!
   Handle individual list items
   */
  template<typename T,
    std::size_t N,
    template<typename, std::size_t>
    typename Container,
    typename =
      std::enable_if_t<std::is_base_of<data::data_reference_base_t, T>::value>>
  void handle(Container<T, N> & list) {
    for(auto & item : list) {
      handle(item);
    }
  }

  /*!
    This method is called on any task arguments that are not handles, e.g.
    scalars or those that did not need any special handling.
   */
  template<typename T>
  void handle(T &) {} // handle

}; // struct task_epilog_t

} // namespace execution
} // namespace flecsi
