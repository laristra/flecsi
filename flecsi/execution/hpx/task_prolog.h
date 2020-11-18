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

#include <queue>
#include <vector>

#include "mpi.h"
#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/common/row_vector.h>
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/execution/context.h>

#include "flecsi/utils/mpi_type_traits.h"
#include <flecsi/utils/tuple_walker.h>
#include <flecsi/utils/type_traits.h>

namespace flecsi {
namespace execution {

/*!
 The task_prolog_t type can be called to walk the task args after the
 task launcher is created, but before the task has run. This allows
 synchronization dependencies to be added to the execution flow.

 @ingroup execution
 */

struct task_prolog_t : public flecsi::utils::tuple_walker_u<task_prolog_t> {

  /*!
   Construct a task_prolog_t instance.
   */

  task_prolog_t() = default;

#if defined(FLECSI_USE_AGGCOMM)
  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(dense_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {

    auto & h = a.handle;
    auto & context = context_t::instance();

    auto rank = context_t::instance().color();

    if(*(h.ghost_is_readable) || (GHOST_PERMISSIONS == na))
      return;

    auto & field_metadata = context.registered_field_metadata().at(h.fid);

    auto & my_coloring_info =
      context.coloring_info(h.index_space).at(context.color());

    auto data = context.registered_field_data().at(h.fid).data();
    auto shared_data = data + my_coloring_info.exclusive * sizeof(T);
    auto ghost_data = shared_data + my_coloring_info.shared * sizeof(T);

    field_metadata.shared_data_buffer = shared_data;
    field_metadata.ghost_data_buffer = ghost_data;
    exchange_queue.emplace(h.index_space, h.fid);
  }

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(ragged_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    if(*(h.ghost_is_readable) || (GHOST_PERMISSIONS == na))
      return;

    update_ghost_row_sizes<T>(h);

    sparse_exchange_queue.emplace(h.index_space, h.fid);
  }

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
  void handle(ragged_mutator<T> & m) {
    auto & h = m.handle;

    if(*(h.ghost_is_readable))
      return;

    update_ghost_row_sizes<T>(h);

    sparse_exchange_queue.emplace(h.index_space, h.fid);
    *(h.ghost_is_readable) = true;
  } // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }
#endif

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor_u<T, PERMISSIONS> & a) {
    if(a.handle.state >= SPECIALIZATION_SPMD_INIT) {
      clog_assert(PERMISSIONS == size_t(ro),
        "you are not allowed "
        "to modify global data in specialization_spmd_init or driver");
    }
  } // handle

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> h) {
    auto & context_ = context_t::instance();

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    int color = static_cast<int>(context_.color());

    // Finish h initialization started in client.h:
    for(size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      const size_t index_space = ent.index_space;
      const size_t dim = ent.dim;
      const size_t domain = ent.domain;

      // get color_info for this field.
      auto & color_info = (context_.coloring_info(index_space)).at(color);
      ent.num_exclusive = color_info.exclusive;
      ent.num_shared = color_info.shared;
      ent.num_ghost = color_info.ghost;

      auto num_entities = ent.num_exclusive + ent.num_shared + ent.num_ghost;

      // see if the field data is registered for this entity field.
      auto & registered_field_data = context_.registered_field_data();
      auto fieldDataIter = registered_field_data.find(ent.fid);
      if(fieldDataIter == registered_field_data.end()) {
        size_t size = ent.size * num_entities;

        execution::context_t::instance().register_field_data(ent.fid, size);
      }
      auto ents = reinterpret_cast<topology::mesh_entity_base_ *>(
        registered_field_data[ent.fid].data());

      fieldDataIter = registered_field_data.find(ent.id_fid);
      if(fieldDataIter == registered_field_data.end()) {
        size_t size = sizeof(utils::id_t) * num_entities;

        execution::context_t::instance().register_field_data(ent.id_fid, size);
      }
      auto ids = reinterpret_cast<utils::id_t *>(
        registered_field_data[ent.id_fid].data());

      // new allocation every time.
      h.storage.init_entities(ent.domain, ent.dim, ents, ids, ent.size,
        num_entities, ent.num_exclusive, ent.num_shared, ent.num_ghost, _read);
    } // for

    for(size_t i{0}; i < h.num_handle_adjacencies; ++i) {
      data_client_handle_adjacency_t & adj = h.handle_adjacencies[i];

      const size_t adj_index_space = adj.adj_index_space;
      const size_t from_index_space = adj.from_index_space;
      const size_t to_index_space = adj.to_index_space;

      auto & color_info = (context_.coloring_info(from_index_space)).at(color);
      auto & registered_field_data = context_.registered_field_data();

      adj.num_offsets =
        (color_info.exclusive + color_info.shared + color_info.ghost);
      auto fieldDataIter = registered_field_data.find(adj.offset_fid);
      if(fieldDataIter == registered_field_data.end()) {
        size_t size = sizeof(size_t) * adj.num_offsets;

        execution::context_t::instance().register_field_data(
          adj.offset_fid, size);
      }
      adj.offsets_buf = reinterpret_cast<size_t *>(
        registered_field_data[adj.offset_fid].data());

      auto adj_info = (context_.adjacency_info()).at(adj_index_space);
      adj.num_indices = adj_info.color_sizes[color];
      fieldDataIter = registered_field_data.find(adj.index_fid);
      if(fieldDataIter == registered_field_data.end()) {
        size_t size = sizeof(utils::id_t) * adj.num_indices;
        execution::context_t::instance().register_field_data(
          adj.index_fid, size);
      }
      adj.indices_buf = reinterpret_cast<utils::id_t *>(
        registered_field_data[adj.index_fid].data());

      h.storage.init_connectivity(adj.from_domain, adj.to_domain, adj.from_dim,
        adj.to_dim, reinterpret_cast<utils::offset_t *>(adj.offsets_buf),
        adj.num_offsets, reinterpret_cast<utils::id_t *>(adj.indices_buf),
        adj.num_indices, _read);
    }

    for(size_t i{0}; i < h.num_index_subspaces; ++i) {
      // get subspace info
      auto & iss = h.handle_index_subspaces[i];
      auto iss_info = (context_.index_subspace_info()).at(iss.index_subspace);
      // the num indices is the capacity (
      auto num_indices = iss_info.capacity;
      // register the field
      auto & registered_field_data = context_.registered_field_data();
      auto fieldDataIter = registered_field_data.find(iss.index_fid);
      if(fieldDataIter == registered_field_data.end()) {
        auto size = sizeof(utils::id_t) * num_indices;
        execution::context_t::instance().register_field_data(
          iss.index_fid, size);
      }
      // assign the storage to the buffer
      iss.indices_buf = reinterpret_cast<utils::id_t *>(
        registered_field_data[iss.index_fid].data());
      // now initialize the index subspace
      h.storage.init_index_subspace(iss.index_space, iss.index_subspace,
        iss.domain, iss.dim, reinterpret_cast<utils::id_t *>(iss.indices_buf),
        num_indices, _read);
    }

    if(!_read) {
      h.initialize_storage();
    }
  } // handle

  /*!
    This method registers entity data fields as needed and initializes set
    topology index spaces and buffers from the raw MPI buffers. If we are
    writing to this buffer, then it sets up the size information of the index
    space as empty so we can call make<>() to push entities onto this buffer.
    If we are reading, this sets up the size as the size recorded in the
    metadata.
   */
  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::set_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> h) {
    auto & context_ = context_t::instance();

    auto & ism = context_.set_index_space_map();

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    int color = context_.color();

    // Finish h initialization started in client.h:
    for(size_t i{0}; i < h.num_handle_entities; ++i) {
      data_client_handle_entity_t & ent = h.handle_entities[i];

      auto iitr = ism.find(ent.index_space);
      clog_assert(iitr != ism.end(), "invalid index space:" << ent.index_space);

      auto citr = iitr->second.color_info_map.find(color);
      clog_assert(
        citr != iitr->second.color_info_map.end(), "invalid color:" << color);
      auto & color_info = citr->second;

      // see if the field data is registered for this entity field.
      auto & registered_field_data = context_.registered_field_data();
      auto fieldDataIter = registered_field_data.find(ent.fid);
      if(fieldDataIter == registered_field_data.end()) {
        size_t size = ent.size * color_info.main_capacity;
        context_.register_field_data(ent.fid, size);

        size = ent.size * color_info.active_migrate_capacity;
        context_.register_field_data(ent.fid2, size);
        context_.register_field_data(ent.fid3, size);
      }

      auto ents = reinterpret_cast<topology::set_entity_t *>(
        registered_field_data[ent.fid].data());

      auto active_ents = reinterpret_cast<topology::set_entity_t *>(
        registered_field_data[ent.fid2].data());

      auto migrate_ents = reinterpret_cast<topology::set_entity_t *>(
        registered_field_data[ent.fid3].data());

      h.storage.init_entities(ent.index_space, ent.index_space2, ents, 0,
        active_ents, 0, migrate_ents, 0, ent.size, _read);
    }
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

  template<typename T>
  void handle(T &) {} // handle

#if defined(FLECSI_USE_AGGCOMM)
  void launch_copies() {
    auto & context = context_t::instance();
    const int my_color = context.color();
    const int num_colors = context.colors();

    auto & ispace_dmap = context.index_space_data_map();

    std::vector<int> sharedSize(num_colors, 0);
    std::vector<int> ghostSize(num_colors, 0);

    std::vector<std::pair<size_t, field_id_t>> modified_fields;

    while(!exchange_queue.empty()) {
      auto & fi = exchange_queue.front();
      auto & field_metadata = context.registered_field_metadata().at(fi.second);
      modified_fields.emplace_back(fi.first, fi.second);

      for(auto rank = 0; rank < num_colors; ++rank) {
        ghostSize[rank] += field_metadata.ghost_field_sizes[rank];
        sharedSize[rank] += field_metadata.shared_field_sizes[rank];
      }

      exchange_queue.pop();
    }

    std::vector<unsigned char *> allSendBuffer(num_colors);
    std::vector<unsigned char *> allRecvBuffer(num_colors);

    std::vector<MPI_Request> allSendRequests(num_colors);
    std::vector<MPI_Request> allRecvRequests(num_colors);

    // Post receives

    for(int rank = 0; rank < num_colors; ++rank) {

      const auto & bufSize = ghostSize[rank];

      if(bufSize == 0) {
        allRecvRequests[rank] = MPI_REQUEST_NULL;
        continue;
      }

      const int resultAlloc =
        MPI_Alloc_mem(bufSize, MPI_INFO_NULL, &allRecvBuffer[rank]);
      if(resultAlloc != MPI_SUCCESS) {
        clog(fatal) << "MPI failed to alloc memory on rank: " << my_color
                    << " with error code: " << resultAlloc << std::endl;
      }

      const int resultRecv = MPI_Irecv(allRecvBuffer[rank], bufSize, MPI_CHAR,
        rank, rank, MPI_COMM_WORLD, &allRecvRequests[rank]);
      if(resultRecv != MPI_SUCCESS) {
        clog(error) << "MPI_Irecv failed on rank " << my_color
                    << " with error code: " << resultRecv << std::endl;
      }
    }

    // pack and send data

    for(int rank = 0; rank < num_colors; ++rank) {

      const auto & bufSize = sharedSize[rank];

      if(bufSize == 0) {
        allSendRequests[rank] = MPI_REQUEST_NULL;
        continue;
      }

      const int resultAlloc =
        MPI_Alloc_mem(bufSize, MPI_INFO_NULL, &allSendBuffer[rank]);
      if(resultAlloc != MPI_SUCCESS) {
        clog(fatal) << "MPI failed to alloc memory on rank: " << my_color
                    << " with error code: " << resultAlloc << std::endl;
      }

      size_t sendBufferOffset = 0;

      for(auto & fi : modified_fields) {
        auto & field_metadata =
          context.registered_field_metadata().at(fi.second);

        for(auto const & ind : field_metadata.shared_indices[rank]) {

          memcpy(&allSendBuffer[rank][sendBufferOffset],
            &field_metadata.shared_data_buffer[ind[0]], ind[1]);
          sendBufferOffset += ind[1];
        }
      }

      const int resultSend = MPI_Isend(allSendBuffer[rank], bufSize, MPI_CHAR,
        rank, my_color, MPI_COMM_WORLD, &allSendRequests[rank]);
      if(resultSend != MPI_SUCCESS) {
        clog(error) << "ERROR: MPI_Isend of rank " << my_color
                    << " for data sent to " << rank
                    << " failed with error code: " << resultSend << std::endl;
      }
    }

    // wait for data to arrive
    const int result = MPI_Waitall(
      allRecvRequests.size(), allRecvRequests.data(), MPI_STATUSES_IGNORE);
    if(result != MPI_SUCCESS) {
      clog(fatal) << "ERROR: MPI_Waitall of rank " << my_color
                  << " on recv requests failed with error code: " << result
                  << std::endl;
    }

    // unpack data
    for(int rank = 0; rank < num_colors; ++rank) {

      const auto & bufSize = ghostSize[rank];

      if(bufSize == 0)
        continue;

      size_t recvBufferOffset = 0;

      for(auto & fi : modified_fields) {

        auto & field_metadata =
          context.registered_field_metadata().at(fi.second);

        for(auto const & ind : field_metadata.ghost_indices[rank]) {

          memcpy(&field_metadata.ghost_data_buffer[ind[0]],
            &allRecvBuffer[rank][recvBufferOffset], ind[1]);
          recvBufferOffset += ind[1];
        }
      }

      MPI_Free_mem(allRecvBuffer[rank]);
    }

    // ensure all send are completed
    MPI_Waitall(
      allSendRequests.size(), allSendRequests.data(), MPI_STATUSES_IGNORE);

    for(int rank = 0; rank < num_colors; ++rank)
      MPI_Free_mem(allSendBuffer[rank]);
  }

  void launch_sparse_copies() {
    auto & context = context_t::instance();
    const int my_color = context.color();
    const int num_colors = context.colors();

    auto & ispace_dmap = context.index_space_data_map();

    std::map<int, int> shared_sizes;
    std::map<int, int> ghost_sizes;
    std::vector<std::pair<size_t, field_id_t>> modified_fields;

    // compute aggregated communication sizes
    while(!sparse_exchange_queue.empty()) {
      auto & fi = sparse_exchange_queue.front();
      auto & field_data = context.registered_sparse_field_data().at(fi.second);
      auto & field_metadata =
        context.registered_sparse_field_metadata().at(fi.second);
      modified_fields.emplace_back(fi.first, fi.second);

      for(const auto & el : field_metadata.ghost_indices) {
        for(size_t ind : el.second) {
          auto count = field_metadata.ghost_row_sizes[ind];
          auto gsize = ghost_sizes.find(el.first);
          if(gsize != ghost_sizes.end())
            gsize->second += (count * field_data.type_size);
          else
            ghost_sizes[el.first] = (count * field_data.type_size);
        }
      }

      auto * rows =
        reinterpret_cast<data::row_vector_u<uint8_t> *>(field_data.rows.data());
      for(const auto & el : field_metadata.shared_indices) {
        for(size_t ind : el.second) {
          const auto & row = rows[field_data.num_exclusive + ind];
          auto count = row.size();
          auto ssize = shared_sizes.find(el.first);
          if(ssize != shared_sizes.end())
            ssize->second += (count * field_data.type_size);
          else
            shared_sizes[el.first] = (count * field_data.type_size);
        }
      }

      sparse_exchange_queue.pop();
    }

    std::map<int, std::vector<uint8_t>> all_send_buf;
    std::map<int, std::vector<uint8_t>> all_recv_buf;
    std::vector<MPI_Request> all_send_req;
    std::vector<MPI_Request> all_recv_req;

    // post recieves
    for(const auto & el : ghost_sizes) {
      int rank = el.first;
      int bufsize = el.second;
      all_recv_buf.emplace(rank, bufsize);
      all_recv_req.push_back(MPI_REQUEST_NULL);
      int err = MPI_Irecv(all_recv_buf[rank].data(), bufsize, MPI_BYTE, rank,
        rank, MPI_COMM_WORLD, &all_recv_req.back());
      if(err != MPI_SUCCESS) {
        clog(error) << "MPI_Irecv failed on rank " << rank
                    << " with error code: " << err << std::endl;
      }
    }

    // pack and send data
    for(const auto & el : shared_sizes) {
      int rank = el.first;
      int bufsize = el.second;
      all_send_buf.emplace(rank, bufsize);
      all_send_req.push_back(MPI_REQUEST_NULL);

      size_t buf_offset = 0;
      for(auto & fi : modified_fields) {
        auto & field_metadata =
          context.registered_sparse_field_metadata().at(fi.second);
        auto & field_data =
          context.registered_sparse_field_data().at(fi.second);
        auto * rows = reinterpret_cast<data::row_vector_u<uint8_t> *>(
          field_data.rows.data());
        for(size_t i : field_metadata.shared_indices[rank]) {
          int r = i + field_data.num_exclusive;
          const auto & row = rows[r];
          size_t count = row.size();
          std::memcpy(&all_send_buf[rank][buf_offset], row.begin(),
            count * field_data.type_size);
          buf_offset += count * field_data.type_size;
        }
      }

      int err = MPI_Isend(all_send_buf[rank].data(), bufsize, MPI_BYTE, rank,
        my_color, MPI_COMM_WORLD, &all_send_req.back());
      if(err != MPI_SUCCESS) {
        clog(error) << "MPI_Isend of rank " << my_color
                    << " with error code: " << err << std::endl;
      }
    }

    // wait for halo data
    {
      int err = MPI_Waitall(
        all_recv_req.size(), all_recv_req.data(), MPI_STATUSES_IGNORE);
      if(err != MPI_SUCCESS) {
        clog(fatal) << "MPI_Waitall "
                    << " on recv requests failed with error code: " << err
                    << std::endl;
      }
    }

    // unpack data
    for(const auto & el : ghost_sizes) {
      int rank = el.first;
      int bufsize = el.second;

      size_t buf_offset = 0;
      for(auto & fi : modified_fields) {
        auto & field_metadata =
          context.registered_sparse_field_metadata().at(fi.second);
        auto & field_data =
          context.registered_sparse_field_data().at(fi.second);
        auto * rows = reinterpret_cast<data::row_vector_u<uint8_t> *>(
          field_data.rows.data());
        for(size_t i : field_metadata.ghost_indices[rank]) {
          int r = field_data.num_exclusive + field_data.num_shared + i;
          auto & row = rows[r];
          int count = field_metadata.ghost_row_sizes[i];
          std::memcpy(row.begin(), &all_recv_buf[rank][buf_offset],
            count * field_data.type_size);
          buf_offset += count * field_data.type_size;
        }
      }
    }

    // wait for sends
    MPI_Waitall(all_send_req.size(), all_send_req.data(), MPI_STATUSES_IGNORE);
  }

  template<typename T, typename HANDLE_TYPE>
  void update_ghost_row_sizes(HANDLE_TYPE & h) {
    auto & context = context_t::instance();
    auto & index_coloring = context.coloring(h.index_space);
    auto & field_metadata =
      context.registered_sparse_field_metadata().at(h.fid);

    // send shared row counts to populate ghost row count list (TODO: aggregate
    // this communication)
    std::map<int, std::vector<uint32_t>> all_send_buf;
    std::map<int, std::vector<uint32_t>> all_recv_buf;
    std::vector<MPI_Request> all_send_req;
    std::vector<MPI_Request> all_recv_req;
    const MPI_Datatype count_mpi_type = utils::mpi_type<std::uint32_t>();

    // post revieves
    for(const auto & el : field_metadata.ghost_indices) {
      int rank = el.first;
      int bufsize = el.second.size();
      all_recv_buf.emplace(rank, bufsize);
      all_recv_req.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(all_recv_buf[rank].data(), bufsize, count_mpi_type, rank, rank,
        MPI_COMM_WORLD, &all_recv_req.back());
    }

    // pack and send data
    for(const auto & el : field_metadata.shared_indices) {
      int rank = el.first;
      all_send_req.push_back(MPI_REQUEST_NULL);
      for(size_t i : field_metadata.shared_indices[rank]) {
        all_send_buf[rank].push_back(h.rows[h.num_exclusive_ + i].size());
      }
      int my_color = context.color();
      MPI_Isend(all_send_buf[rank].data(), all_send_buf[rank].size(),
        count_mpi_type, rank, my_color, MPI_COMM_WORLD, &all_send_req.back());
    }

    // wait for row sizes
    MPI_Waitall(all_recv_req.size(), all_recv_req.data(), MPI_STATUSES_IGNORE);

    for(const auto & el : field_metadata.ghost_indices) {
      int rank = el.first;
      int buf_offset = 0;
      for(size_t i : field_metadata.ghost_indices[rank]) {
        field_metadata.ghost_row_sizes[i] = all_recv_buf[rank][buf_offset];
        int r = h.num_exclusive_ + h.num_shared_ + i;
        auto & row = h.rows[r];
        row.resize(field_metadata.ghost_row_sizes[i]);
        buf_offset++;
      }
    }

    // wait for sends
    MPI_Waitall(all_send_req.size(), all_send_req.data(), MPI_STATUSES_IGNORE);
  }

  std::queue<std::pair<size_t, field_id_t>> exchange_queue;
  std::queue<std::pair<size_t, field_id_t>> sparse_exchange_queue;
#endif

}; // struct task_prolog_t

} // namespace execution
} // namespace flecsi
