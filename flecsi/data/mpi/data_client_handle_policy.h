/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include <flecsi/runtime/types.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! FIXME: Description of class

//----------------------------------------------------------------------------//

struct data_client_handle_entity_t
{
  using field_id_t = size_t;

  data_client_handle_entity_t() : index_space(0), dim(0), domain(0), size(0), fid(0) {}

  size_t index_space;
  size_t dim;
  size_t domain;
  size_t size;
  size_t num_exclusive;
  size_t num_shared;
  size_t num_ghost;
  field_id_t fid;
  field_id_t id_fid;
}; // struct data_client_handle_entity_t

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//
struct data_client_handle_adjacency_t
{
  size_t adj_index_space;
  size_t from_index_space;
  size_t to_index_space;
  size_t from_domain;
  size_t to_domain;
  size_t from_dim;
  size_t to_dim;
  size_t num_offsets;
  size_t num_indices;
  field_id_t index_fid;
  field_id_t offset_fid;
  size_t * offsets_buf;
  uint64_t * indices_buf;
};

struct mpi_data_client_handle_policy_t
{
  // FIXME: This needs to be exposed at a higher level
  static constexpr size_t MAX_ADJACENCIES = 20;
  static constexpr size_t MAX_ENTITIES = 5;

  size_t num_handle_entities;
  size_t num_handle_adjacencies;
  data_client_handle_entity_t handle_entities[MAX_ENTITIES];
  data_client_handle_adjacency_t handle_adjacencies[MAX_ADJACENCIES];
}; // struct data_client_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
