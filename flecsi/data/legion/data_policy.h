/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_data_policy_h
#define flecsi_data_legion_data_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include "flecsi/data/storage.h"

#include "flecsi/data/legion/global.h"
#include "flecsi/data/legion/dense.h"
#include "flecsi/data/legion/sparse.h"
#include "flecsi/data/legion/scoped.h"
#include "flecsi/data/legion/tuple.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! The legion data policy defines types that are specific to the Legion
//! runtime.
//----------------------------------------------------------------------------//

struct legion_data_policy_t
{

  //--------------------------------------------------------------------------//
  //! The storage_type__ type determines the underlying storage mechanism
  //! for the backend runtime.
  //--------------------------------------------------------------------------//

  template<
    size_t STORAGE_TYPE
  >
  using storage_type__ = legion::storage_type__<STORAGE_TYPE>;

}; // class legion_data_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_legion_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
