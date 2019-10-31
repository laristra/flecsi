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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "../entity_storage.hh"
#include "../index_space.hh"
#include "flecsi/runtime/backend.hh"
#include <flecsi/runtime/context.hh>
#include <flecsi/topology/ntree/storage.hh>
#include <flecsi/topology/ntree/types.hh>
#include <flecsi/utils/id.hh>

#include <array>

namespace flecsi {
namespace topology {

template<class TREE_TYPE>
struct ntree_storage {

  using id_t = utils::id_t;
  static constexpr size_t num_partitions = 5;

  using entity_t = typename TREE_TYPE::entity_t;
  using node_t = typename TREE_TYPE::node_t;
  using tree_entity_t = typename TREE_TYPE::tree_entity_t;

  // entity index spaces
  using entity_index_space_t =
    index_space<entity_t *, true, true, true, void, topology_storage>;
  using entity_index_subspaces_t =
    index_space<entity_t *, false, true, false, void, topology_storage>;
  using entity_partition_index_spaces_t =
    index_space<entity_t *, false, false, true, void, topology_storage>;

  entity_index_space_t entity_index_space;
  entity_index_subspaces_t entity_index_subspaces;
  std::array<entity_partition_index_spaces_t, num_partitions>
    entity_partition_index_spaces;

  // Tree entity index space
  using tree_entity_index_space_t =
    index_space<tree_entity_t *, true, true, true, void, topology_storage>;
  using tree_entity_index_subspaces_t =
    index_space<tree_entity_t *, false, true, false, void, topology_storage>;
  using tree_entity_partition_index_spaces_t =
    index_space<tree_entity_t *, false, false, true, void, topology_storage>;

  tree_entity_index_space_t tree_entity_index_space;
  tree_entity_index_subspaces_t tree_entity_index_subspaces;
  std::array<tree_entity_partition_index_spaces_t, num_partitions>
    tree_entity_partition_index_spaces;

  // Nodes index space
  using node_index_space_t =
    index_space<node_t *, true, true, true, void, topology_storage>;
  using node_index_subspaces_t =
    index_space<node_t *, false, true, false, void, topology_storage>;
  using node_partition_index_spaces_t =
    index_space<node_t *, false, false, true, void, topology_storage>;

  node_index_space_t node_index_space;
  node_index_subspaces_t node_index_subspaces;
  std::array<node_partition_index_spaces_t, num_partitions>
    node_partition_index_spaces;

  ntree_storage() {
    // auto & context_ = flecsi::runtime::context_t::instance();
  }

  void finalize_storage() {
    // auto & context = runtime::context_t::instance();
  }

  /**
   * @brief Initialize the entitiy index space
   */
  void init_entities(entity_t * buf_entity,
    id_t * buf_id_entity,
    tree_entity_t * buf_tree_entity,
    id_t * buf_id_tree_entity,
    size_t num_entities) {
    entity_index_space.storage()->set_buffer(buf_entity, num_entities);
    entity_index_space.id_storage().set_buffer(buf_id_entity, num_entities);
    tree_entity_index_space.storage()->set_buffer(
      buf_tree_entity, num_entities);
    tree_entity_index_space.id_storage().set_buffer(
      buf_id_tree_entity, num_entities);
  }

  /**
   * Initilize the node index space
   */
  void init_nodes(node_t * buf, size_t num_nodes) {
    node_index_space.storage()->set_buffer(buf, num_nodes);
  }

  /**
   * Build an entity for the user
   */
  template<class... ARG_TYPES>
  entity_t * make_entity(ARG_TYPES &&... args) {
    size_t entity = entity_index_space.size();
    auto placement_ptr =
      static_cast<entity_t *>(entity_index_space.storage()->buffer()) + entity;
    auto ent = new(placement_ptr) entity_t(std::forward<ARG_TYPES>(args)...);
    id_t global_id = id_t::make<0, 0>(entity);
    ent->template set_global_id(global_id);
    auto & id_storage = entity_index_space.id_storage();
    id_storage[entity] = global_id;
    entity_index_space.pushed();
    return ent;
  } // make

}; // class mpi_topology_storage_policy

} // namespace topology
} // namespace flecsi
