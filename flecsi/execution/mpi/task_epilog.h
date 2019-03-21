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

    using accessor_t = ragged_accessor<T, EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS, GHOST_PERMISSIONS>;
    using handle_t = typename accessor_t::handle_t;
    using offset_t = typename handle_t::offset_t;
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

    value_t * entries = h.entries;
    auto offsets = &(h.offsets)[0];
    auto shared_data = entries + h.reserve;
    auto ghost_data = shared_data + h.num_shared_ * h.max_entries_per_index;

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
          offsets[h.num_exclusive_ + shared.offset].count());
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
      offsets[h.num_exclusive_ + h.num_shared_ + i].set_count(
        recv_count_buf[i]);
    }
  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(sparse_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    using base_t = typename sparse_accessor<T, EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS, GHOST_PERMISSIONS>::base_t;
    handle(static_cast<base_t &>(a));
  } // handle

  template<typename T>
  void handle(ragged_mutator<T> & m) {
    auto & h = m.h_;

    using mutator_t = ragged_mutator_u<T>;
    using handle_t = typename mutator_t::handle_t;
    using offset_t = typename handle_t::offset_t;
    using value_t = T;
    using commit_info_t = typename handle_t::commit_info_t;

    clog_assert(*h.num_exclusive_insertions <= *h.reserve,
      "sparse exclusive reserve exceed");

    // this segfaults if we try to use a sparse mutator more than once
    // delete h.num_exclusive_insertions;

    value_t * entries = reinterpret_cast<value_t *>(&(*h.entries)[0]);

    commit_info_t ci;
    ci.offsets = &(*h.offsets)[0];
    ci.entries[0] = entries;
    ci.entries[1] = entries + *h.reserve;
    ci.entries[2] = ci.entries[1] + h.num_shared() * h.max_entries_per_index();

    h.commit(&ci);

  } // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    using base_t = typename sparse_mutator<T>::base_t;
    handle(static_cast<base_t &>(m));
  }

  /*!
   Handle individual list items
   */
  template<
    typename T,
    std::size_t N,
    template<typename, std::size_t> typename Container,
    typename = std::enable_if_t< std::is_base_of<data::data_reference_base_t, T>::value >
  >
  void handle( Container<T, N> & list ) {
    for ( auto & item : list ) {
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
