/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include <flecsi/runtime/types.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! Provides a collection of fields which are populated when traversing the
//! data client entity information which is then passed to the data client
//! handle.
//----------------------------------------------------------------------------//

struct data_client_handle_entity_t {
  size_t index_space;
  // topological dimension
  size_t dim;
  // topological domain
  size_t domain;
  size_t size;
  size_t num_exclusive;
  size_t num_shared;
  size_t num_ghost;
  field_id_t fid;
  field_id_t id_fid;
  Legion::LogicalRegion color_region;
  Legion::LogicalRegion exclusive_region;
  Legion::LogicalRegion shared_region;
  Legion::LogicalRegion ghost_region;
}; // struct data_client_handle_entity_t

//----------------------------------------------------------------------------//
//! Provides a collection of fields which are populated when, in the case of
//! mesh topology, traversing the data client adjacency specifications,
//! which is then passed the data client handle.
//----------------------------------------------------------------------------//

struct data_client_handle_adjacency_t {
  size_t adj_index_space;
  size_t from_index_space;
  size_t to_index_space;
  size_t from_domain;
  size_t to_domain;
  // from the topological dimension
  size_t from_dim;
  // to topological dimension
  size_t to_dim;
  size_t num_offsets;
  size_t num_indices;
  field_id_t index_fid;
  field_id_t offset_fid;
  Legion::LogicalRegion adj_region;
  Legion::LogicalRegion from_color_region;
  Legion::LogicalRegion from_primary_region;
}; // struct data_client_handle_adjacency_t

struct data_client_handle_index_subspace_t {
  size_t index_space;
  size_t index_subspace;
  field_id_t index_fid;
  Legion::LogicalRegion region;
};

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct legion_data_client_handle_policy_t {

  // FIXME: This needs to be exposed at a higher level
  
  // maximum number of adjacencies to read, this limits the size of the
  // serialize struct passed to Legion
  static constexpr size_t MAX_ADJACENCIES = 20;
  
  // maximum number of handle entities
  static constexpr size_t MAX_ENTITIES = 5;

  // maximum number of handle index subspaces
  static constexpr size_t MAX_INDEX_SUBSPACES = 10;

  size_t num_handle_entities;
  size_t num_handle_adjacencies;
  size_t num_index_subspaces;
  data_client_handle_entity_t handle_entities[MAX_ENTITIES];
  data_client_handle_adjacency_t handle_adjacencies[MAX_ADJACENCIES];
  data_client_handle_index_subspace_t handle_index_subspaces[MAX_INDEX_SUBSPACES];
}; // struct data_client_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
