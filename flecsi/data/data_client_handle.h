/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_data_client_handle_h
#define flecsi_data_data_client_handle_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<
  typename DATA_CLIENT_TYPE,
  typename DATA_POLICY
>
struct data_client_handle_base__ : public DATA_CLIENT_TYPE, public DATA_POLICY
{
  using field_id_t = Legion::FieldID;

  static constexpr size_t MAX_ADJACENCIES = 20;

  size_t num_adjacencies;
  size_t adj_index_spaces[MAX_ADJACENCIES];
  size_t from_index_spaces[MAX_ADJACENCIES];
  size_t to_index_spaces[MAX_ADJACENCIES];
  field_id_t index_fids[MAX_ADJACENCIES];
  field_id_t entity_fids[MAX_ADJACENCIES];
  field_id_t offset_fids[MAX_ADJACENCIES];
  Legion::LogicalRegion adj_regions[MAX_ADJACENCIES];
  Legion::LogicalRegion color_regions[MAX_ADJACENCIES];
}; // struct data_client_handle__

} // namespace flecsi

#include "flecsi_runtime_data_client_handle_policy.h"

namespace flecsi {

//----------------------------------------------------------------------------//
//! The data_handle__ type is the high-level data handle type.
//!
//! @tparam DATA_CLIENT_TYPE The client type.
//! @tparam DATA_POLICY      The data policy for this handle type.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<
  typename DATA_CLIENT_TYPE  
>
using data_client_handle__ = data_client_handle_base__<
  DATA_CLIENT_TYPE,
  FLECSI_RUNTIME_DATA_CLIENT_HANDLE_POLICY
>;

} // namespace flecsi

#endif // flecsi_data_data_client_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
