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


#include <vector>

#include "mpi.h"
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/execution/context.h>
#include <flecsi/coloring/mpi_utils.h>

namespace flecsi {
namespace execution {

  /*!
   The task_prolog_t type can be called to walk the task args after the
   task launcher is created, but before the task has run. This allows
   synchronization dependencies to be added to the execution flow.
  
   @ingroup execution
   */

  struct task_prolog_t : public utils::tuple_walker__<task_prolog_t>
  {

    /*!
     Construct a task_prolog_t instance.
     */

    task_prolog_t() = default;

    /*!
     FIXME: Need a description.
    
     @tparam T                     The data type referenced by the handle.
     @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                                   indices of the index partition.
     @tparam SHARED_PERMISSIONS    The permissions required on the shared
                                   indices of the index partition.
     @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                                   indices of the index partition.
    
     @param runtime The Legion task runtime.
     */

    template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS
    >
    void
    handle(
     dense_accessor__<
       T,
       EXCLUSIVE_PERMISSIONS,
       SHARED_PERMISSIONS,
       GHOST_PERMISSIONS
     > & a
    )
    {
      // TODO: move field data allocation here?
    } // handle

    template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS
    >
    void
    handle(
      sparse_accessor<
        T,
        EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS,
        GHOST_PERMISSIONS
      > & a
    )
    {
//      // TODO: move field data allocation here?
//      auto& context = context_t::instance();
//      const int my_color = context.color();
//      auto& my_coloring_info =
//        context.coloring_info(h.index_space).at(my_color);
//
//      auto& sparse_field_metadata =
//        context.registered_sparse_field_metadata().at(h.fid);
//
//     for (auto i = my_coloring_info.exclusive;
//          i < my_coloring_info.exclusive + my_coloring_info.shared; ++i) {
//       h()
//     }
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
      m.h_.init();
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
      m.h_.init();
    } // handle

    template<
      typename T,
      size_t PERMISSIONS
    >
    typename std::enable_if_t<std::is_base_of<topology::mesh_topology_base_t, T>::value>
    handle(
      data_client_handle__<T, PERMISSIONS> & h
    )
    {
      auto& context_ = context_t::instance();

      // h is partially initialized in client.h
      auto storage = h.set_storage(new typename T::storage_t);

      bool _read{ PERMISSIONS == ro || PERMISSIONS == rw };

      int color = context_.color();

      for(size_t i{0}; i<h.num_handle_entities; ++i) {
        data_client_handle_entity_t & ent = h.handle_entities[i];

        const size_t index_space = ent.index_space;
        const size_t dim = ent.dim;
        const size_t domain = ent.domain;

        // get color_info for this field.
        auto& color_info = (context_.coloring_info(index_space)).at(color);
        ent.num_exclusive = color_info.exclusive;
        ent.num_shared = color_info.shared;
        ent.num_ghost = color_info.ghost;

        auto num_entities = ent.num_exclusive + ent.num_shared + ent.num_ghost;

        // see if the field data is registered for this entity field.
        auto& registered_field_data = context_.registered_field_data();
        auto fieldDataIter = registered_field_data.find(ent.fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = ent.size * num_entities;

          execution::context_t::instance().register_field_data(ent.fid,
                                                               size);
        }
        auto ents =
          reinterpret_cast<topology::mesh_entity_base_*>(registered_field_data[ent.fid].data());

        fieldDataIter = registered_field_data.find(ent.id_fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = ent.size * num_entities;

          execution::context_t::instance().register_field_data(ent.id_fid,
                                                               size);
        }
        auto ids =
          reinterpret_cast<utils::id_t *>(registered_field_data[ent.id_fid].data());

        // new allocation every time.
        storage->init_entities(ent.domain, ent.dim,
                               ents, ids, ent.size,
                               num_entities, ent.num_exclusive,
                               ent.num_shared, ent.num_ghost,
                               _read);
      } // for

      for(size_t i{0}; i<h.num_handle_adjacencies; ++i) {
        data_client_handle_adjacency_t &adj = h.handle_adjacencies[i];

        const size_t adj_index_space = adj.adj_index_space;
        const size_t from_index_space = adj.from_index_space;
        const size_t to_index_space = adj.to_index_space;

        auto& color_info = (context_.coloring_info(from_index_space)).at(color);
        auto& registered_field_data = context_.registered_field_data();

        adj.num_offsets = (color_info.exclusive + color_info.shared + color_info.ghost);
        auto fieldDataIter = registered_field_data.find(adj.offset_fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = sizeof(size_t) * adj.num_offsets;

          execution::context_t::instance().register_field_data(adj.offset_fid,
                                                               size);
        }
        adj.offsets_buf = reinterpret_cast<size_t *>(registered_field_data[adj.offset_fid].data());

        auto adj_info = (context_.adjacency_info()).at(adj_index_space);
        adj.num_indices = adj_info.color_sizes[color];
        fieldDataIter = registered_field_data.find(adj.index_fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = sizeof(utils::id_t) * adj.num_indices;
          execution::context_t::instance().register_field_data(adj.index_fid,
                                                               size);
        }
        adj.indices_buf = reinterpret_cast<size_t *>(registered_field_data[adj.index_fid].data());

        storage->init_connectivity(adj.from_domain, adj.to_domain,
                                   adj.from_dim, adj.to_dim,
                                   reinterpret_cast<utils::offset_t *>(adj.offsets_buf),
                                   adj.num_offsets,
                                   reinterpret_cast<utils::id_t *>(adj.indices_buf),
                                   adj.num_indices,
                                   _read);
      }

      if(!_read){
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
    template<
      typename T,
      size_t PERMISSIONS
    >
    typename std::enable_if_t<std::is_base_of<topology::set_topology_base_t, T>::value>
    handle(
      data_client_handle__<T, PERMISSIONS> & h
    )
    {
      auto& context_ = context_t::instance();

      // h is partially initialized in client.h
      auto storage = h.set_storage(new typename T::storage_t);

      bool _read{ PERMISSIONS == ro || PERMISSIONS == rw };

      int color = context_.color();

      auto& im = context_.local_index_space_data_map();

      for(size_t i{0}; i<h.num_handle_entities; ++i) {
        data_client_handle_entity_t & ent = h.handle_entities[i];

        auto itr = im.find(ent.index_space);
        clog_assert(itr != im.end(),
          "invalid local index space: " << ent.index_space);
        const context_t::local_index_space_data_t& isd = itr->second;

        // see if the field data is registered for this entity field.
        auto& registered_field_data = context_.registered_field_data();
        auto fieldDataIter = registered_field_data.find(ent.fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = ent.size * isd.capacity;
          context_.register_field_data(ent.fid, size);
        }

        auto ents =
          reinterpret_cast<topology::set_entity_t*>(
          registered_field_data[ent.fid].data());

        storage->init_entities(ent.index_space, ents, ent.size, isd.size, _read);
      }
    }

    template<
      typename T
    >
    static
    typename std::enable_if_t<
		!std::is_base_of<dense_data_handle_base_t, T>::value>
    handle(
      T&
    )
    {
    } // handle

  }; // struct task_prolog_t

} // namespace execution
} // namespace flecsi
