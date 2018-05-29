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
// the same convention, e.g., -DFLECSI_DEPENDENT_PARTITION_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Legion Policy
#if FLECSI_DEPENDENT_PARTITION_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/execution/legion/dependent_partition_policy.h>

namespace flecsi {
namespace execution {

using FLECSI_DEPENDENT_PARTITION_POLICY = legion_dependent_partition_policy_t;

} // namespace execution
} // namespace flecsi

// MPI Policy
#elif FLECSI_DEPENDENT_PARTITION_MODEL == FLECSI_RUNTIME_MODEL_mpi

/*
#include <flecsi/execution/mpi/dependent_partition_policy.h>

namespace flecsi {
namespace execution {

using FLECSI_DEPENDENT_PARTITION_POLICY = mpi_dependent_partition_policy_t;

} // namespace execution
} // namespace flecsi
*/

#endif // FLECSI_DEPENDENT_PARTITION_MODEL