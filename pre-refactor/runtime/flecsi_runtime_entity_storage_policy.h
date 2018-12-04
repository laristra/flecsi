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

#include <flecsi/topology/common/entity_storage.h>

namespace flecsi {

template<typename T>
using FLECSI_RUNTIME_ENTITY_STORAGE_TYPE = topology::topology_storage_u<T>;

using FLECSI_RUNTIME_OFFSET_STORAGE_TYPE = topology::offset_storage_;

} // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/topology/common/entity_storage.h>

namespace flecsi {

template<typename T>
using FLECSI_RUNTIME_ENTITY_STORAGE_TYPE = topology::topology_storage_u<T>;

using FLECSI_RUNTIME_OFFSET_STORAGE_TYPE = topology::offset_storage_;

} // namespace flecsi

// HPX Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/topology/common/entity_storage.h>

namespace flecsi {

template<typename T>
using FLECSI_RUNTIME_ENTITY_STORAGE_TYPE = topology::topology_storage_u<T>;

using FLECSI_RUNTIME_OFFSET_STORAGE_TYPE = topology::offset_storage_;

} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
