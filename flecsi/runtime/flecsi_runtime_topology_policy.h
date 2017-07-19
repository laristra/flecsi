/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_topology_policy_h
#define flecsi_runtime_topology_policy_h

//----------------------------------------------------------------------------//
// @file
// @date Initial file creation: Jun 19, 2017
//----------------------------------------------------------------------------//

#include <flecsi.h>

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Serial Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

  #include "flecsi/topology/serial/storage_policy.h"

  namespace flecsi {
  template <size_t ND, size_t NM>
  using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY = 
    topology::serial_topology_storage_policy_t<ND, NM>;

  }

// Legion, MPI+Legion Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/topology/legion/storage_policy.h"

  namespace flecsi {

  template <size_t ND, size_t NM>
  using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY = 
    topology::legion_topology_storage_policy_t<ND, NM>;

  }

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #include "flecsi/topology/mpi/storage_policy.h"

  namespace flecsi {
  template <size_t ND, size_t NM>
  using FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY = 
    topology::mpi_topology_storage_policy_t<ND, NM>;

  }

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_topology_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
