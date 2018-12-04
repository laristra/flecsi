/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Legion Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/data/legion/data_client_handle_policy.h>

namespace flecsi {

using FLECSI_RUNTIME_DATA_CLIENT_HANDLE_POLICY =
  legion_data_client_handle_policy_t;

} // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/data/mpi/data_client_handle_policy.h>

namespace flecsi {

using FLECSI_RUNTIME_DATA_CLIENT_HANDLE_POLICY =
  mpi_data_client_handle_policy_t;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/data/hpx/data_client_handle_policy.h>

namespace flecsi {

using FLECSI_RUNTIME_DATA_CLIENT_HANDLE_POLICY =
  hpx_data_client_handle_policy_t;

} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
