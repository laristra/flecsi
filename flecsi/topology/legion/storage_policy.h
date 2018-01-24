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

#include <array>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <legion/arrays.h>
#include <legion_stl.h>
#include <legion_utilities.h>

#include <flecsi/execution/context.h>
#include <flecsi/topology/common/entity_storage.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/topology/mesh_storage.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/utils/id.h>

namespace flecsi {
namespace topology {

template<size_t, size_t>
class mesh_entity__;

///
/// \class legion_data_handle_policy_t data_handle_policy.h
/// \brief legion_data_handle_policy_t provides...
///

template<size_t NUM_DIMS, size_t NUM_DOMAINS, size_t NUM_INDEX_SUBSPACES>
struct legion_topology_storage_policy_t__ {
  static constexpr size_t num_partitions = 5;

  using id_t = utils::id_t;

  using index_spaces_t = std::array<
      index_space__<
          mesh_entity_base_ *,
          true,
          true,
          true,
          void,
          topology_storage__>,
      NUM_DIMS + 1>;

  using index_subspaces_t = std::array<
      index_space__<
          mesh_entity_base_ *,
          true,
          true,
          true,
          void,
          topology_storage__>,
      NUM_INDEX_SUBSPACES>;

  using partition_index_spaces_t = std::array<
      index_space__<
          mesh_entity_base_ *,
          false,
          false,
          true,
          void,
          topology_storage__>,
      NUM_DIMS + 1>;

  // array of array of domain_connectivity__
  std::array<std::array<domain_connectivity__<NUM_DIMS>, NUM_DOMAINS>,
    NUM_DOMAINS> topology;

  std::array<index_spaces_t, NUM_DOMAINS> index_spaces;

  std::array<index_spaces_t, NUM_INDEX_SUBSPACES> subindex_spaces;

  std::array<std::array<partition_index_spaces_t, NUM_DOMAINS>, num_partitions>
      partition_index_spaces;

  size_t color;

  legion_topology_storage_policy_t__() {
    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();
  }

  void init_entities(
      size_t domain,
      size_t dim,
      mesh_entity_base_ * entities,
      utils::id_t * ids,
      size_t size,
      size_t num_entities,
      size_t num_exclusive,
      size_t num_shared,
      size_t num_ghost,
      bool read) {
    auto & is = index_spaces[domain][dim];

    auto s = is.storage();
    s->set_buffer(entities, num_entities, read);

    auto & id_storage = is.id_storage();
    id_storage.set_buffer(ids, num_entities, true);

    for (auto & domain_connectivities : topology) {
      auto & domain_connectivity__ = domain_connectivities[domain];
      for (size_t d = 0; d <= NUM_DIMS; ++d) {
        domain_connectivity__.get(d, dim).set_entity_storage(s);
      } // for
    } // for

    if (!read) {
      return;
    }

    is.set_end(num_entities);

    size_t shared_end = num_exclusive + num_shared;
    size_t ghost_end = shared_end + num_ghost;

    for (size_t partition = 0; partition < num_partitions; ++partition) {
      auto & isp = partition_index_spaces[partition][domain][dim];
      isp.set_storage(s);
      isp.set_id_storage(&id_storage);

      switch (partition_t(partition)) {
        case exclusive:
          isp.set_begin(0);
          isp.set_end(num_exclusive);
          break;
        case shared:
          isp.set_begin(num_exclusive);
          isp.set_end(shared_end);
          break;
        case ghost:
          isp.set_begin(shared_end);
          isp.set_end(ghost_end);
          break;
        case owned:
          isp.set_begin(0);
          isp.set_end(shared_end);
          break;
        default:
          break;
      }
    }
  } // init_entities

  void init_subentities(
      size_t index_space,
      mesh_entity_base_ * entities,
      utils::id_t * ids,
      size_t size,
      size_t num_entities,
      bool read) {
    auto & is = subindex_spaces[index_space];

    auto s = is.storage();
    s->set_buffer(entities, num_entities, read);

    auto & id_storage = is.id_storage();
    id_storage.set_buffer(ids, num_entities, true);

    if (!read) {
      return;
    }

    is.set_end(num_entities);
  } // init_subentities

  void init_connectivity(
      size_t from_domain,
      size_t to_domain,
      size_t from_dim,
      size_t to_dim,
      utils::offset_t * offsets,
      size_t num_offsets,
      utils::id_t * indices,
      size_t num_indices,
      bool read) {
    // TODO - this is an initial implementation for testing purposes.
    // We may wish to store the buffer pointers coming from Legion directly
    // into the connectivity

    auto & conn = topology[from_domain][to_domain].get(from_dim, to_dim);

    auto & id_storage = conn.get_index_space().id_storage();
    id_storage.set_buffer(indices, num_indices, read);

    conn.offsets().storage().set_buffer(offsets, num_offsets, read);

    if (read) {
      conn.get_index_space().set_end(num_indices);
    }
  } // init_connectivities

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make(ARG_TYPES &&... args) {
    using dtype = domain_entity__<DOM, T>;

    auto & is = index_spaces[DOM][T::dimension].template cast<dtype>();
    size_t entity = is.size();

    auto placement_ptr = static_cast<T *>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<ARG_TYPES>(args)...);

    id_t global_id = id_t::make<T::dimension, DOM>(entity, color);
    ent->template set_global_id<DOM>(global_id);

    auto & id_storage = is.id_storage();

    id_storage[entity] = global_id;

    is.pushed();

    return ent;
  } // make

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make(const id_t & id, ARG_TYPES &&... args) {
    using dtype = domain_entity__<DOM, T>;

    auto & is = index_spaces[DOM][T::dimension].template cast<dtype>();

    size_t entity = id.entity();

    auto placement_ptr = static_cast<T *>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<ARG_TYPES>(args)...);

    ent->template set_global_id<DOM>(id);

    auto & id_storage = is.id_storage();

    id_storage[entity] = id;

    is.pushed();

    return ent;
  } // make

}; // class legion_topology_storage_policy_t__

} // namespace topology
} // namespace flecsi
