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

// Legion, MPI+Legion Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/data/legion/dense_data_handle_policy.h>
#include <flecsi/data/legion/global_data_handle_policy.h>
#include <flecsi/data/legion/sparse_data_handle_policy.h>

namespace flecsi {

using FLECSI_RUNTIME_DENSE_DATA_HANDLE_POLICY =
  legion_dense_data_handle_policy_t;

using FLECSI_RUNTIME_GLOBAL_DATA_HANDLE_POLICY =
  legion_global_data_handle_policy_t;

using FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY =
  legion_sparse_data_handle_policy_t;

} // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/data/mpi/data_handle_policy.h>
#include <flecsi/data/mpi/sparse_data_handle_policy.h>

namespace flecsi {

using FLECSI_RUNTIME_DENSE_DATA_HANDLE_POLICY = mpi_data_handle_policy_t;

using FLECSI_RUNTIME_GLOBAL_DATA_HANDLE_POLICY = mpi_data_handle_policy_t;

using FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY =
  mpi_sparse_data_handle_policy_t;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/data/hpx/data_handle_policy.h>
#include <flecsi/data/hpx/sparse_data_handle_policy.h>

namespace flecsi {

using FLECSI_RUNTIME_DENSE_DATA_HANDLE_POLICY = hpx_data_handle_policy_t;

using FLECSI_RUNTIME_GLOBAL_DATA_HANDLE_POLICY = hpx_data_handle_policy_t;

using FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY =
  hpx_sparse_data_handle_policy_t;

} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
