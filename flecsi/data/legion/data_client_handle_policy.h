/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_data_client_handle_policy_h
#define flecsi_data_legion_data_client_handle_policy_h

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
  size_t index_space;
  size_t dim;
  size_t domain;
  size_t size;
  field_id_t fid;
  Legion::LogicalRegion color_region;
}; // struct data_client_handle_entity

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct data_client_handle_adjacency
{
  size_t adj_index_space;
  size_t from_index_space;
  size_t to_index_space;
  size_t from_domain;
  size_t to_domain;
  size_t from_dim;
  size_t to_dim;
  size_t num_offsets;;
  size_t num_indices;;
  field_id_t index_fid;
  field_id_t offset_fid;
  Legion::LogicalRegion adj_region;
  Legion::LogicalRegion from_color_region;
  Legion::LogicalRegion from_primary_region;
  LegionRuntime::Arrays::Point<2> * offsets_buf;
  uint64_t * indices_buf;
};

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct legion_data_client_handle_policy_t
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

#endif // flecsi_data_legion_data_client_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
