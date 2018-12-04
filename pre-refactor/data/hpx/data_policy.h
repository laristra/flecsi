/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_hpx_data_policy_h
#define flecsi_data_hpx_data_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include "flecsi/data/hpx/registration_wrapper.h"
#include "flecsi/data/storage.h"

#include "flecsi/data/hpx/dense.h"
#include "flecsi/data/hpx/global.h"
#include "flecsi/data/hpx/scoped.h"
#include "flecsi/data/hpx/sparse.h"
#include "flecsi/data/hpx/tuple.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct hpx_data_policy_t {
  template<size_t STORAGE_TYPE>
  using storage_class_u = hpx::storage_class_u<STORAGE_TYPE>;

  template<
      typename DATA_CLIENT_TYPE,
      size_t STORAGE_TYPE,
      typename DATA_TYPE,
      size_t NAMESPACE_HASH,
      size_t NAME_HASH,
      size_t INDEX_SPACE,
      size_t VERSIONS>
  using field_wrapper_u = hpx_field_registration_wrapper_u<
      DATA_CLIENT_TYPE,
      STORAGE_TYPE,
      DATA_TYPE,
      NAMESPACE_HASH,
      NAME_HASH,
      INDEX_SPACE,
      VERSIONS>;

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  using client_wrapper_u = hpx_client_registration_wrapper_u<
      DATA_CLIENT_TYPE,
      NAMESPACE_HASH,
      NAME_HASH>;

}; // class hpx_data_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_hpx_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
