/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <flecsi/data/common/data_types.h>
#include <vector>
#include <cstddef>
#include <cstdint>

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The hpx_mutator_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct hpx_mutator_handle_policy_t
{
  using offset_t = data::sparse_data_offset_t;

  hpx_mutator_handle_policy_t(){}

  hpx_mutator_handle_policy_t(const hpx_mutator_handle_policy_t& p) = default;

  field_id_t fid;
  std::size_t index_space;
  std::size_t data_client_hash;

  // offset_t is start offset and count
  std::vector<offset_t>* offsets;
  // an entry is entry ID and data type value, this buffer is sized according
  // to the size print entry times total number of entries
  std::vector<std::uint8_t>* entries;
  // the reserve includes the current number of allocated exclusive
  // entries plus those unused
  std::size_t* reserve;
  // the current simple allocation strategy will allocate at least
  // exclusive_reserve more entries when when resizing reserve
  std::size_t exclusive_reserve;
  // the current number of actually used exclusive entries within the
  // reserve buffer
  std::size_t* num_exclusive_entries;
}; // class hpx_mutator_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
*~-------------------------------------------------------------------------~-*/
