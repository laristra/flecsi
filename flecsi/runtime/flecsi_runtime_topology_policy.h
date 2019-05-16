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

#include <flecsi/topology/legion/storage_policy.h>

namespace flecsi {

template<size_t NUM_DIMS, size_t NUM_DOMAINS, size_t NUM_INDEX_SUBSPACES>
using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY =
  topology::legion_topology_storage_policy_t_u<NUM_DIMS,
    NUM_DOMAINS,
    NUM_INDEX_SUBSPACES>;

} // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/topology/mpi/storage_policy.h>

namespace flecsi {

template<size_t NUM_DIMS, size_t NUM_DOMAINS, size_t NUM_INDEX_SUBSPACES>
using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY = topology::
  mpi_topology_storage_policy_u<NUM_DIMS, NUM_DOMAINS, NUM_INDEX_SUBSPACES>;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include "flecsi/topology/hpx/storage_policy.h"

namespace flecsi {

template<size_t NUM_DIMS, size_t NUM_DOMAINS, size_t NUM_INDEX_SUBSPACES>
using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY = topology::
  hpx_topology_storage_policy_u<NUM_DIMS, NUM_DOMAINS, NUM_INDEX_SUBSPACES>;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include "flecsi/topology/hpx/storage_policy.h"

namespace flecsi {

template<size_t ND, size_t NM>
using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY =
  topology::hpx_topology_storage_policy_u<ND, NM>;

} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
