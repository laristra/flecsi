/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_data_policy_h
#define flecsi_runtime_data_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

#include "flecsi.h"

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Serial Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

  #include "flecsi/data/default_user_meta_data.h"
  #include "flecsi/data/serial/storage_policy.h"

  namespace flecsi {
  namespace data {

  using FLECSI_RUNTIME_USER_META_DATA_POLICY = default_user_meta_data_t;

  template<typename META_DATA>
  using FLECSI_RUNTIME_STORAGE_POLICY = serial_storage_policy_t<META_DATA>;

  } // namespace data
  } // namespace flecsi

//Legion Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/data/default_user_meta_data.h"
  #include "flecsi/data/legion/storage_policy.h"

  namespace flecsi {
  namespace data {

  using FLECSI_RUNTIME_USER_META_DATA_POLICY = default_user_meta_data_t;

  template<typename META_DATA>
  using FLECSI_RUNTIME_STORAGE_POLICY = legion_storage_policy_t<META_DATA>;

  } // namespace data
  } // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #include "flecsi/data/default_user_meta_data.h"
  #include "flecsi/data/mpi/storage_policy.h"

  namespace flecsi {
  namespace data {

  using FLECSI_RUNTIME_USER_META_DATA_POLICY = default_user_meta_data_t;

  template<typename META_DATA>
  using FLECSI_RUNTIME_STORAGE_POLICY = mpi_storage_policy_t<META_DATA>;

  } // namespace data
  } // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
