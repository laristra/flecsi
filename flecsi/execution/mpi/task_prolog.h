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
#include <flecsi/coloring/mpi_utils.h>
#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/execution/context.h>

#include <flecsi/utils/tuple_walker.h>

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
  handle(data_client_handle_u<T, PERMISSIONS> & h) {
    auto & context_ = context_t::instance();

    // h is partially initialized in client.h
    auto storage = h.set_storage(new typename T::storage_t);

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    int color = context_.color();

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
        size_t size = ent.size * num_entities;

        execution::context_t::instance().register_field_data(ent.id_fid, size);
      }
      auto ids = reinterpret_cast<utils::id_t *>(
        registered_field_data[ent.id_fid].data());

      // new allocation every time.
      storage->init_entities(ent.domain, ent.dim, ents, ids, ent.size,
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
      adj.indices_buf =
        reinterpret_cast<id_t *>(registered_field_data[adj.index_fid].data());

      storage->init_connectivity(adj.from_domain, adj.to_domain, adj.from_dim,
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
      iss.indices_buf =
        reinterpret_cast<id_t *>(registered_field_data[iss.index_fid].data());
      // now initialize the index subspace
      storage->init_index_subspace(iss.index_space, iss.index_subspace,
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
  handle(data_client_handle_u<T, PERMISSIONS> & h) {
    auto & context_ = context_t::instance();

    auto & ism = context_.set_index_space_map();

    // h is partially initialized in client.h
    auto storage = h.set_storage(new typename T::storage_t);

    bool _read{PERMISSIONS == ro || PERMISSIONS == rw};

    int color = context_.color();

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

      storage->init_entities(ent.index_space, ent.index_space2, ents, 0,
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

  template<typename T>
  void handle(T &) {} // handle

}; // struct task_prolog_t

} // namespace execution
} // namespace flecsi
