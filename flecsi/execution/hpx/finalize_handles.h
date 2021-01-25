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

#include <cstring>
#include <stdint.h>
#include <vector>

#include "mpi.h"

#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/topology/mesh_topology.h>
#include <flecsi/topology/set_topology.h>
#include <flecsi/utils/mpi_type_traits.h>

#include <flecsi/utils/tuple_walker.h>
#include <flecsi/utils/type_traits.h>

namespace flecsi {
namespace execution {

struct finalize_handles_t
  : public flecsi::utils::tuple_walker_u<finalize_handles_t> {

  template<typename T>
  void handle(ragged_mutator<T> & m) {
    auto & h = m.handle;

    using value_t = T;

#if !defined(FLECSI_USE_AGGCOMM)
    auto & context = context_t::instance();
    const int my_color = static_cast<int>(context.color());
    auto & my_coloring_info = context.coloring_info(h.index_space).at(my_color);
    auto index_coloring = context.coloring(h.index_space);

    auto & sparse_field_metadata =
      context.registered_sparse_field_metadata().at(h.fid);

    value_t * shared_data =
      new value_t[h.num_shared() * h.max_entries_per_index];
    value_t * ghost_data = new value_t[h.num_ghost() * h.max_entries_per_index];

    // Load data into shared data buffer
    for(int i = 0; i < h.num_shared(); ++i) {
      int r = static_cast<int>(h.num_exclusive_ + i);
      const auto & row = h.rows[r];
      size_t count = row.size();
      std::memcpy(&shared_data[i * h.max_entries_per_index], row.begin(),
        count * sizeof(value_t));
    } // for i

    // Get entry_values
    MPI_Datatype shared_ghost_type;
    MPI_Type_contiguous(sizeof(value_t), MPI_BYTE, &shared_ghost_type);
    MPI_Type_commit(&shared_ghost_type);

    MPI_Win win;
    MPI_Win_create(shared_data,
      sizeof(value_t) * h.num_shared() * h.max_entries_per_index,
      sizeof(value_t), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    MPI_Win_post(sparse_field_metadata.shared_users_grp, 0, win);
    MPI_Win_start(sparse_field_metadata.ghost_owners_grp, 0, win);

    int i = 0;
    for(auto & ghost : index_coloring.ghost) {
      clog_rank(warn, 0) << "ghost id: " << ghost.id << ", rank: " << ghost.rank
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

    // for (int i = 0; i < h.num_ghost() * h.max_entries_per_index; i++)
    //  clog_rank(warn, 0) << "ghost after: " << ghost_data[i].value <<
    //  std::endl;

    int send_count = 0;
    for(auto & shared : index_coloring.shared) {
      send_count += static_cast<int>(shared.shared.size());
    }

    // Send/Recv counts in entry_values.
    std::vector<MPI_Request> requests(send_count);
    std::vector<MPI_Status> statuses(send_count);
    std::vector<MPI_Request> recv_requests(h.num_ghost());
    std::vector<MPI_Status> recv_status(h.num_ghost());

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
        MPI_Isend(&send_count_buf[i], 1,
          flecsi::utils::mpi_typetraits_u<uint32_t>::type(),
          static_cast<int>(peer), 99, MPI_COMM_WORLD, &requests[i]);
        i++;
      }
    }

    std::vector<uint32_t> recv_count_buf(h.num_ghost());
    i = 0;
    for(auto & ghost : index_coloring.ghost) {
      // MPI_Status status;
      MPI_Irecv(&recv_count_buf[i], 1,
        flecsi::utils::mpi_typetraits_u<uint32_t>::type(),
        static_cast<int>(ghost.rank), 99, MPI_COMM_WORLD, &recv_requests[i]);
      i++;
    }

    MPI_Waitall(send_count, requests.data(), statuses.data());
    MPI_Waitall(static_cast<int>(h.num_ghost()), recv_requests.data(),
      recv_status.data());

    for(int i = 0; i < h.num_ghost(); i++) {
      clog_rank(warn, 0) << recv_count_buf[i] << std::endl;
    }

    // Unload data from ghost data buffer
    for(int i = 0; i < static_cast<int>(h.num_ghost()); i++) {
      int r = static_cast<int>(h.num_exclusive_ + h.num_shared() + i);
      auto & row = h.rows[r];
      int count = recv_count_buf[i];
      row.resize(count);
      std::memcpy(row.begin(), &ghost_data[i * h.max_entries_per_index],
        count * sizeof(value_t));
    }

    delete[] shared_data;
    delete[] ghost_data;
#else
    *(h.ghost_is_readable) = false;
#endif
  } // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }

  /*!
    Finalize set topology storage. This inspects index based sizes and
    writes out appropriate metadata.
   */
  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::set_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> h) {
    h.storage.finalize_storage();
  } // handle

  /*!
   The finalize_handles_t type can be called to walk task args after task
   execution. This allows us to free memory allocated during the task.

   @ingroup execution
   */

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> h) {
    if(PERMISSIONS == wo || PERMISSIONS == rw) {
      auto & context_ = context_t::instance();
      auto & ssm = context_.index_subspace_info();

      for(size_t i{0}; i < h.num_index_subspaces; ++i) {
        data_client_handle_index_subspace_t & iss = h.handle_index_subspaces[i];

        auto itr = ssm.find(iss.index_subspace);
        clog_assert(itr != ssm.end(), "invalid index subspace");
        auto & si = itr->second;

        clog_assert(si.size == 0, "index subspace size already set");
        si.size = h.get_index_subspace_size_(iss.index_subspace);
      } // for
    } // if
  } // handle

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
    for(auto & item : list)
      handle(item);
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

  //-----------------------------------------------------------------------//
  // If this is not a data handle, then simply skip it.
  //-----------------------------------------------------------------------//

  template<typename T>
  void handle(T &) {} // handle

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi
