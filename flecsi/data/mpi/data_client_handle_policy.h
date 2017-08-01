/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_mpi_data_client_handle_policy_h
#define flecsi_data_mpi_data_client_handle_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include "flecsi/runtime/types.h"

namespace flecsi {

//----------------------------------------------------------------------------//
//! FIXME: Description of class

//----------------------------------------------------------------------------//
struct data_client_handle_entity
{
  using field_id_t = size_t;

  size_t index_space;
  size_t dim;
  size_t domain;
  size_t size;
  field_id_t fid;
//  Legion::LogicalRegion color_region;
}; // struct data_client_handle_entity

struct data_client_handle_adjacency
{
  size_t adj_index_space;
  size_t from_index_space;
  size_t to_index_space;
  size_t from_domain;
  size_t to_domain;
  size_t from_dim;
  size_t to_dim;
  field_id_t index_fid;
  field_id_t entity_fid;
  field_id_t offset_fid;
};

struct mpi_data_client_handle_policy_t
{
  // FIXME: This needs to be exposed at a higher level
  static constexpr size_t MAX_ADJACENCIES = 20;
  static constexpr size_t MAX_ENTITIES = 5;

  size_t num_handle_entities;
  size_t num_handle_adjacencies;
  data_client_handle_entity handle_entities[MAX_ENTITIES];
  data_client_handle_adjacency handle_adjacencies[MAX_ADJACENCIES];
}; // struct data_client_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_mpi_data_client_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
