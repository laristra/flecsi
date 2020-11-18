/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

#include <flecsi/execution/hpx/future.h>
#include <flecsi/runtime/types.h>

namespace flecsi {

//----------------------------------------------------------------------------//

struct hpx_data_handle_policy_t {
  // +++ The following fields are set from get_handle(), reading
  // information from the context which is data that is the same
  // across multiple ranks/colors and should be used ONLY as read-only data

  field_id_t fid;
  size_t index_space;
  size_t data_client_hash;
  bool * ghost_is_readable;

  execution::hpx_future_u<void> future;

}; // class mpi_data_handle_policy_t

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
