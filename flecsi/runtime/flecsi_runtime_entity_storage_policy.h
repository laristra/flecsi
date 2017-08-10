/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_entity_storage_policy_h
#define flecsi_runtime_entity_storage_policy_h

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

  #include "flecsi/topology/serial/entity_storage.h"

  namespace flecsi {

  template<typename T>
  using FLECSI_RUNTIME_ENTITY_STORAGE_TYPE = topology::entity_storage__<T>;

  }

// Legion, MPI+Legion Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/topology/legion/entity_storage.h"

  namespace flecsi {

  template<typename T>
  using FLECSI_RUNTIME_ENTITY_STORAGE_TYPE = topology::entity_storage__<T>;

  }

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #include "flecsi/topology/mpi/entity_storage.h"

  namespace flecsi {

  template<typename T>
  using FLECSI_RUNTIME_ENTITY_STORAGE_TYPE = topology::entity_storage__<T>;

  }

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_entity_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
