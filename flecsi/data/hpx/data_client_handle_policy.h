/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <cstddef>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include <flecsi/execution/hpx/future.h>
#include <flecsi/runtime/types.h>
#include <flecsi/topology/mesh_types.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! Provides a collection of fields which are populated when traversing the
//! data client entity information which is then passed to the data client
//! handle.
//----------------------------------------------------------------------------//

struct data_client_handle_entity_t {
  using field_id_t = std::size_t;

  data_client_handle_entity_t()
    : index_space(0), dim(0), domain(0), size(0), fid(0) {}

  std::size_t index_space;
  std::size_t index_space2;
  std::size_t dim;
  std::size_t domain;
  std::size_t size;
  std::size_t num_exclusive;
  std::size_t num_shared;
  std::size_t num_ghost;
  field_id_t fid;
  field_id_t fid2;
  field_id_t fid3;
  field_id_t id_fid;
  std::size_t fid_size;
  std::size_t fid2_size;
  std::size_t fid3_size;
  std::size_t id_fid_size;
}; // struct data_client_handle_entity_t

//----------------------------------------------------------------------------//
//! Provides a collection of fields which are populated when, in the case of
//! mesh topology, traversing the data client adjacency specifications,
//! which is then passed the data client handle.
//----------------------------------------------------------------------------//
struct data_client_handle_adjacency_t {
  using field_id_t = std::size_t;

  std::size_t adj_index_space;
  std::size_t from_index_space;
  std::size_t to_index_space;
  std::size_t from_domain;
  std::size_t to_domain;
  std::size_t from_dim;
  std::size_t to_dim;
  std::size_t num_offsets;
  std::size_t num_indices;
  field_id_t index_fid;
  field_id_t offset_fid;
  std::size_t index_fid_size;
  std::size_t offset_fid_size;
  std::size_t * offsets_buf;
  utils::id_t * indices_buf;
};

struct data_client_handle_index_subspace_t {
  using field_id_t = std::size_t;

  std::size_t index_space;
  std::size_t index_subspace;
  field_id_t index_fid;
  std::size_t index_fid_size;
  utils::id_t * indices_buf;
  std::size_t domain;
  std::size_t dim;
};

struct hpx_data_client_handle_policy_t {
  // FIXME: This needs to be exposed at a higher level

  // maximum number of adjacencies to read, this limits the size of the
  // serialize struct passed to Legion
  // static constexpr size_t MAX_ADJACENCIES = 32;
  static constexpr std::size_t MAX_ADJACENCIES = 64;

  // maximum number of handle entities
  static constexpr std::size_t MAX_ENTITIES = 7;

  // maximum number of handle index subspaces
  static constexpr std::size_t MAX_INDEX_SUBSPACES = 10;

  std::size_t num_handle_entities;
  std::size_t num_handle_adjacencies;
  std::size_t num_index_subspaces;
  data_client_handle_entity_t handle_entities[MAX_ENTITIES];
  data_client_handle_adjacency_t handle_adjacencies[MAX_ADJACENCIES];
  data_client_handle_index_subspace_t
    handle_index_subspaces[MAX_INDEX_SUBSPACES];

  execution::hpx_future_u<void> future;

}; // struct data_client_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
