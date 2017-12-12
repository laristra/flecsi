/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_mutator_handle_policy_h
#define flecsi_data_legion_mutator_handle_policy_h

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include "flecsi/runtime/types.h"

//----------------------------------------------------------------------------//
/// @file
/// @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The legion_mutator_handle_policy_t type provides backend storage for
//! interfacing to the Legion runtime.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct legion_mutator_handle_policy_t {
  legion_mutator_handle_policy_t() {}

  legion_mutator_handle_policy_t(const legion_mutator_handle_policy_t & p) =
      default;

}; // class legion_mutator_handle_policy_t

} // namespace flecsi

#endif // flecsi_data_legion_mutator_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
