/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_hpx_data_client_handle_policy_h
#define flecsi_data_hpx_data_client_handle_policy_h

#include <cstddef>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

namespace flecsi {
//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//
struct data_client_handle_entity_t {
  // FIXME check context
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
};

//----------------------------------------------------------------------------//
//! Provides a collection of fields which are populated when, in the case of
//! mesh topology, traversing the data client adjacency specifications,
//! which is then passed the data client handle.
//----------------------------------------------------------------------------//
struct data_client_handle_adjacency_t {
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
  std::size_t * offsets_buf;
  uint64_t * indices_buf;
};

struct data_client_handle_index_subspace_t {
  std::size_t index_space;
  std::size_t index_subspace;
  field_id_t index_fid;
  uint64_t * indices_buf;
  std::size_t domain;
  std::size_t dim;
};

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct hpx_data_client_handle_policy_t {}; // struct data_client_handle_policy_t
// data_client_handle_index_subspace_t
// handle_index_subspaces[MAX_INDEX_SUBSPACES];

} // namespace flecsi

#endif // flecsi_data_hpx_data_client_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
