/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <flecsi/data/common/data_types.h>

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The mpi_mutator_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct mpi_mutator_handle_policy_t
{
  using offset_t = data::sparse_data_offset_t;

  mpi_mutator_handle_policy_t(){}

  mpi_mutator_handle_policy_t(const mpi_mutator_handle_policy_t& p) = default;

  field_id_t fid;
  size_t index_space;
  size_t data_client_hash;

  // offset_t is start offset and count
  std::vector<offset_t>* offsets;
  // an entry is entry ID and data type value, this buffer is sized according
  // to the size print entry times total number of entries
  std::vector<uint8_t>* entries;
  // the reserve includes the current number of allocated exclusive
  // entries plus those unused
  size_t* reserve;
  // the current simple allocation strategy will allocate at least
  // reserve_chunk more entries when when resizing reserve
  size_t reserve_chunk;
  // the current number of actually used exclusive entries within the
  // reserve buffer
  size_t* num_exclusive_entries;
}; // class mpi_mutator_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
