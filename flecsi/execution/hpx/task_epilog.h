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
#include <flecsi/data/global_accessor.h>
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

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/context.h>

#include "flecsi/utils/mpi_type_traits.h"
#include <flecsi/utils/tuple_walker.h>
#include <flecsi/utils/type_traits.h>

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
#if !defined(FLECSI_USE_AGGCOMM)
    // Skip Read Only handles
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      return;
    }
    else {
      auto & context = context_t::instance();
      const int my_color = static_cast<int>(context.color());
      auto & my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);

      auto & field_metadata = context.registered_field_metadata().at(h.fid);

      MPI_Win win = field_metadata.win;

      MPI_Win_post(field_metadata.shared_users_grp, 0, win);
      MPI_Win_start(field_metadata.ghost_owners_grp, 0, win);

      for(auto owner : my_coloring_info.ghost_owners) {
        int ghost_owner = static_cast<int>(owner);
        MPI_Get(h.ghost_data, 1, field_metadata.origin_types[ghost_owner],
          ghost_owner, 0, 1, field_metadata.target_types[ghost_owner], win);
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);

    } // else
#else
    auto & context = context_t::instance();

    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo))
      *(h.ghost_is_readable) = true;
    else if(SHARED_PERMISSIONS == rw || SHARED_PERMISSIONS == wo)
      *(h.ghost_is_readable) = false;
#endif
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor_u<T, PERMISSIONS> & a) {
    auto & h = a.handle;

    // Skip Read Only handles
    if(PERMISSIONS == ro)
      return;

    auto & context = context_t::instance();
    const int my_color = static_cast<int>(context.color());
    MPI_Bcast(
      &a.data(), static_cast<int>(sizeof(T)), MPI_BYTE, 0, MPI_COMM_WORLD);
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

#if !defined(FLECSI_USE_AGGCOMM)
    // Skip Read Only handles
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      return;
    }
    else {
      auto & context = context_t::instance();
      const int my_color = static_cast<int>(context.color());
      auto & my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);
      auto index_coloring = context.coloring(h.index_space);

      auto & sparse_field_metadata =
        context.registered_sparse_field_metadata().at(h.fid);

      value_t * shared_data =
        new value_t[h.num_shared_ * h.max_entries_per_index];
      value_t * ghost_data =
        new value_t[h.num_ghost_ * h.max_entries_per_index];

      // Load data into shared data buffer
      for(int i = 0; i < h.num_shared_; ++i) {
        int r = static_cast<int>(i + h.num_exclusive_);
        const auto & row = h.rows[r];
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
        clog_rank(warn, 0) << "ghost id: " << ghost.id
                           << ", rank: " << ghost.rank
                           << ", offset: " << ghost.offset << std::endl;
        MPI_Get(&ghost_data[i * h.max_entries_per_index],
          static_cast<int>(h.max_entries_per_index), shared_ghost_type,
          static_cast<int>(ghost.rank), ghost.offset * h.max_entries_per_index,
          static_cast<int>(h.max_entries_per_index), shared_ghost_type, win);
        i++;
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);

      MPI_Win_free(&win);
      MPI_Type_free(&shared_ghost_type);

      for(int i = 0; i < h.num_ghost_ * h.max_entries_per_index; i++)
        clog_rank(warn, 0) << "ghost after: " << ghost_data[i] << std::endl;

      int send_count = 0;
      for(auto & shared : index_coloring.shared) {
        send_count += static_cast<int>(shared.shared.size());
      }

      // Send/Recv counts in entry_values.
      std::vector<MPI_Request> requests(send_count + h.num_ghost_);
      std::vector<MPI_Status> statuses(send_count + h.num_ghost_);

      const MPI_Datatype count_mpi_type = utils::mpi_type<std::uint32_t>();
      std::vector<uint32_t> send_count_buf;
      for(auto & shared : index_coloring.shared) {
        for(auto peer : shared.shared) {
          send_count_buf.push_back(
            h.rows[h.num_exclusive_ + shared.offset].size());
        }
      }

      i = 0;
      for(auto & shared : index_coloring.shared) {
        for(auto peer : shared.shared) {
          MPI_Isend(&send_count_buf[i], 1, count_mpi_type,
            static_cast<int>(peer), static_cast<int>(shared.id), MPI_COMM_WORLD,
            &requests[i]);
          i++;
        }
      }

      std::vector<uint32_t> recv_count_buf(h.num_ghost_);
      i = 0;
      for(auto & ghost : index_coloring.ghost) {
        // MPI_Status status;
        MPI_Irecv(&recv_count_buf[i], 1, count_mpi_type,
          static_cast<int>(ghost.rank), static_cast<int>(ghost.id),
          MPI_COMM_WORLD, &requests[i + send_count]);
        i++;
      }

      MPI_Waitall(static_cast<int>(send_count + h.num_ghost_), requests.data(),
        statuses.data());

      for(int i = 0; i < h.num_ghost_; i++) {
        clog_rank(warn, 0) << recv_count_buf[i] << std::endl;
      }

      // Unload data from ghost data buffer
      for(int i = 0; i < static_cast<int>(h.num_ghost_); i++) {
        int r = static_cast<int>(h.num_exclusive_ + h.num_shared_ + i);
        auto & row = h.rows[r];
        int count = recv_count_buf[i];
        row.resize(count);
        std::memcpy(row.begin(), &ghost_data[i * h.max_entries_per_index],
          count * sizeof(value_t));
      }

      delete[] shared_data;
      delete[] ghost_data;
    } // else
#else
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      *(h.ghost_is_readable) = true;
    }
    else {
      *(h.ghost_is_readable) = false;
    }
#endif
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

  template<size_t I, typename T, size_t PERMISSIONS>
  void client_handler(data_client_handle_u<T, PERMISSIONS> h) {

    using entity_types_t = typename T::types_t::entity_types;

    if constexpr(I < std::tuple_size<entity_types_t>::value) {

      // get the entitiy type
      using entity_tuple_t =
        typename std::tuple_element<I, entity_types_t>::type;
      using entity_type_t =
        typename std::tuple_element<2, entity_tuple_t>::type;
      constexpr auto DIM = entity_type_t::dimension;
      constexpr auto DOM = entity_type_t::domain;

      // mpi stats
      int comm_size, comm_rank;
      MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

      // loop over entities and exchange the ghost values
      auto entity_size = sizeof(entity_type_t);
      auto entities = h.template get_entities<DIM, DOM>();

      // get context information
      auto & context = context_t::instance();
      const int my_color = static_cast<int>(context.color());

      // figure out index space id
      constexpr auto index_space = topology::find_index_space_from_dimension_u<
        std::tuple_size<entity_types_t>::value, entity_types_t, DIM,
        DOM>::find();
      const auto & index_map = context.index_map(index_space);

      // get ghost/shared info
      const auto & my_coloring = context.coloring(index_space);
      const auto & my_coloring_info =
        context.coloring_info(index_space).at(my_color);

      // entity offsets are relative to the start of shared
      auto offset_start = my_coloring_info.exclusive;
      entities += offset_start;

      // allocate send and receive buffers
      using byte_t = unsigned char;

      // setup send buffers
      std::vector<size_t> sendcounts(comm_size, 0);
      for(auto & shared : my_coloring.shared) {
        for(auto peer : shared.shared) {
          assert(peer != comm_rank);
          sendcounts[peer] += entity_size;
        }
      }

      std::vector<size_t> senddispls(comm_size + 1);
      senddispls[0] = 0;
      for(size_t r = 0; r < comm_size; ++r)
        senddispls[r + 1] = senddispls[r] + sendcounts[r];

      std::fill(sendcounts.begin(), sendcounts.end(), 0);
      std::vector<byte_t> sendbuf(senddispls[comm_size], 0);
      for(auto & shared : my_coloring.shared) {
        auto eptr = &entities[shared.offset];
        for(auto peer : shared.shared) {
          auto offset = senddispls[peer] + sendcounts[peer];
          std::memcpy(sendbuf.data() + offset, eptr, entity_size);
          sendcounts[peer] += entity_size;
        }
      }

      // setup recv buffers
      std::vector<size_t> recvcounts(comm_size, 0);
      for(auto & ghost : my_coloring.ghost)
        recvcounts[ghost.rank] += entity_size;

      std::vector<size_t> recvdispls(comm_size + 1);

      recvdispls[0] = 0;
      for(size_t r = 0; r < comm_size; ++r)
        recvdispls[r + 1] = recvdispls[r] + recvcounts[r];

      std::vector<byte_t> recvbuf(recvdispls[comm_size]);

      // exchange data
      auto ret = coloring::alltoallv(sendbuf, sendcounts, senddispls, recvbuf,
        recvcounts, recvdispls, MPI_COMM_WORLD);
      if(ret != MPI_SUCCESS) {
        clog_error("Error communicating vertices");
      }

      // unpack data
      constexpr auto num_domains = T::num_domains;

      size_t i{0};
      std::fill(recvcounts.begin(), recvcounts.end(), 0);

      for(auto & ghost : my_coloring.ghost) {
        // get pointer to entity in question
        auto offset = recvdispls[ghost.rank] + recvcounts[ghost.rank];
        auto eptr = entities + my_coloring_info.shared + i;
        // copy the original ids for now (GROSS)
        auto id = eptr->global_id();
        // overrite data
        std::memcpy(eptr, recvbuf.data() + offset, entity_size);
        // copy back the ids
        eptr->set_global_id(id);
        // bump counters
        recvcounts[ghost.rank] += entity_size;
        ++i;
      }

      // recursively call this function
      client_handler<I + 1>(h);
    } // constexpr if
  }

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> h) {

    // skip read only
    if(PERMISSIONS == ro)
      return;

    // iterate over types
    client_handler<0>(h);
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
   * Handle tuple of items
   */

  template<typename... Ts, size_t... I>
  void handle_tuple_items(std::tuple<Ts...> & items,
    std::index_sequence<I...>) {
    (handle(std::get<I>(items)), ...);
  }

  template<typename... Ts,
    typename = std::enable_if_t<
      utils::are_base_of_t<data::data_reference_base_t, Ts...>::value>>
  void handle(std::tuple<Ts...> & items) {
    handle_tuple_items(items, std::make_index_sequence<sizeof...(Ts)>{});
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
