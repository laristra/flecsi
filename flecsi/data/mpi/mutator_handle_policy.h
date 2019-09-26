/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <flecsi/data/common/data_types.h>
#include <vector>

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

struct mpi_mutator_handle_policy_t {
  using offset_t = data::sparse_data_offset_t;

  mpi_mutator_handle_policy_t() {}

  mpi_mutator_handle_policy_t(const mpi_mutator_handle_policy_t & p) = default;

  field_id_t fid;
}; // class mpi_mutator_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
