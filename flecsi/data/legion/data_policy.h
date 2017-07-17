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

#include "flecsi/data/legion/registration_wrapper.h"
#include "flecsi/data/storage.h"

#include "flecsi/data/legion/global.h"
#include "flecsi/data/legion/dense.h"
#include "flecsi/data/legion/sparse.h"
#include "flecsi/data/legion/scoped.h"
#include "flecsi/data/legion/tuple.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct legion_data_policy_t
{
  template<
    size_t STORAGE_TYPE
  >
  using storage_type__ = legion::storage_type__<STORAGE_TYPE>;

  template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSIONS,
    size_t INDEX_SPACE
  >
  using field_wrapper__ = legion_field_registration_wrapper__<
    DATA_CLIENT_TYPE,
    STORAGE_TYPE,
    DATA_TYPE,
    NAMESPACE_HASH,
    NAME_HASH,
    VERSIONS,
    INDEX_SPACE
  >;

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  using client_wrapper__ = legion_client_registration_wrapper__<
    DATA_CLIENT_TYPE,
    NAMESPACE_HASH,
    NAME_HASH
  >;

}; // class legion_data_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_legion_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
