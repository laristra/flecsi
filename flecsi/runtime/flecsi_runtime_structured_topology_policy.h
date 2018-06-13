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

#include <flecsi/topology/legion/structured_storage_policy.h>

namespace flecsi {

template<size_t NUM_DIMS, size_t NUM_DOMAINS>
using FLECSI_RUNTIME_STRUCTURED_TOPOLOGY_STORAGE_POLICY =
      topology::legion_structured_topology_storage_policy_t__<
      NUM_DIMS,
      NUM_DOMAINS>;

} // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/topology/mpi/structured_storage_policy.h>

namespace flecsi {

template<size_t NUM_DIMS, size_t NUM_DOMAINS>
using FLECSI_RUNTIME_STRUCTURED_TOPOLOGY_STORAGE_POLICY = 
      topology::mpi_structured_topology_storage_policy__<
      NUM_DIMS, 
      NUM_DOMAINS>;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include "flecsi/topology/hpx/structured_storage_policy.h"

namespace flecsi {

template<size_t NUM_DIMS, size_t NUM_DOMAINS>
using FLECSI_RUNTIME_STRUCTURED_TOPOLOGY_STORAGE_POLICY = 
      topology::hpx_structured_topology_storage_policy__<
      NUM_DIMS, 
      NUM_DOMAINS>;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include "flecsi/topology/hpx/storage_policy.h"

namespace flecsi {

template<size_t ND, size_t NM>
using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY =
    topology::hpx_topology_storage_policy__<ND, NM>;

} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
