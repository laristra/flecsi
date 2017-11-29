/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_set_topology_policy_h
#define flecsi_runtime_set_topology_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 19, 2017
//----------------------------------------------------------------------------//

#include <flecsi-config.h>

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Legion, MPI+Legion Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/topology/legion/set_storage_policy.h"

  namespace flecsi {

  template<typename SET_TYPES>
  using FLECSI_RUNTIME_SET_TOPOLOGY_STORAGE_POLICY = 
    topology::legion_set_topology_storage_policy_t<SET_TYPES>;

  } // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #include "flecsi/topology/mpi/set_storage_policy.h"

  namespace flecsi {

  template<typename SET_TYPES>
  using FLECSI_RUNTIME_SET_TOPOLOGY_STORAGE_POLICY = 
    topology::mpi_set_topology_storage_policy_t<SET_TYPES>;

  } // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_set_topology_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
