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
// implemenation for the storage model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Legion Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/data/legion/storage_policy.h>

namespace flecsi {
namespace data {

using FLECSI_RUNTIME_STORAGE_POLICY = legion_storage_policy_t;

} // namespace data
} // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/data/mpi/storage_policy.h>

namespace flecsi {
namespace data {

using FLECSI_RUNTIME_STORAGE_POLICY = mpi_storage_policy_t;

} // namespace data
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/data/hpx/storage_policy.h>

namespace flecsi {
namespace data {

using FLECSI_RUNTIME_STORAGE_POLICY = hpx_storage_policy_t;

} // namespace data
} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
