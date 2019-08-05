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
#else
#include "flecsi/runtime/context_policy.hh"
#include <flecsi/topology/common/entity_storage.hh>
#include <flecsi/topology/common/index_space.hh>
#include <flecsi/topology/ntree/storage.hh>
#include <flecsi/topology/ntree/types.hh>
#include <flecsi/utils/id.hh>
#endif

#include <array>

namespace flecsi {
namespace topology {

template<class TREE_TYPE>
struct ntree_storage_u {

  using id_t = utils::id_t;
  static constexpr size_t num_partitions = 5;

  using entity_t = typename TREE_TYPE::tree_entity_;
  using branch_t = typename TREE_TYPE::tree_branch_;
  using tree_entity_t = typename TREE_TYPE::tree_entity_holder_;

  // entity index spaces
  using entity_index_space_t =
    index_space_u<entity_t *, true, true, true, void, topology_storage_u>;
  using entity_index_subspaces_t =
    index_space_u<entity_t *, false, true, false, void, topology_storage_u>;
  using entity_partition_index_spaces_t =
    index_space_u<entity_t *, false, false, true, void, topology_storage_u>;

  entity_index_space_t entity_index_space;
  entity_index_subspaces_t entity_index_subspaces;
  std::array<entity_partition_index_spaces_t, num_partitions>
    entity_partition_index_spaces;

  // Tree entity index space
  using tree_entity_index_space_t =
    index_space_u<tree_entity_t *, true, true, true, void, topology_storage_u>;
  using tree_entity_index_subspaces_t = index_space_u<tree_entity_t *,
    false,
    true,
    false,
    void,
    topology_storage_u>;
  using tree_entity_partition_index_spaces_t = index_space_u<tree_entity_t *,
    false,
    false,
    true,
    void,
    topology_storage_u>;

  tree_entity_index_space_t tree_entity_index_space;
  tree_entity_index_subspaces_t tree_entity_index_subspaces;
  std::array<tree_entity_partition_index_spaces_t, num_partitions>
    tree_entity_partition_index_spaces;

  // Branches index space
  using branch_index_space_t =
    index_space_u<branch_t *, true, true, true, void, topology_storage_u>;
  using branch_index_subspaces_t =
    index_space_u<branch_t *, false, true, false, void, topology_storage_u>;
  using branch_partition_index_spaces_t =
    index_space_u<branch_t *, false, false, true, void, topology_storage_u>;

  branch_index_space_t branch_index_space;
  branch_index_subspaces_t branch_index_subspaces;
  std::array<branch_partition_index_spaces_t, num_partitions>
    branch_partition_index_spaces;

  ntree_storage_u() {
    auto & context_ = flecsi::execution::context_t::instance();
  }

  void finalize_storage() {
    auto & context = execution::context_t::instance();
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
   * Initilize the branch index space
   */
  void init_branches(branch_t * buf, size_t num_branches) {
    branch_index_space.storage()->set_buffer(buf, num_branches);
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

}; // class mpi_topology_storage_policy_u

} // namespace topology
} // namespace flecsi
