/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_data_handle_policy_h
#define flecsi_runtime_data_handle_policy_h

//----------------------------------------------------------------------------//
// @file
// @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

#include "flecsi.h"

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

// Serial Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

  #include "flecsi/data/serial/data_handle_policy.h"

  namespace flecsi {

  using FLECSI_RUNTIME_DATA_HANDLE_POLICY = serial_data_handle_policy_t;

  }

// Legion, MPI+Legion Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/data/legion/data_handle_policy.h"

  namespace flecsi {

  using FLECSI_RUNTIME_DATA_HANDLE_POLICY = legion_data_handle_policy_t;

  }

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #include "flecsi/data/mpi/data_handle_policy.h"

  namespace flecsi {

  using FLECSI_RUNTIME_DATA_HANDLE_POLICY = mpi_data_handle_policy_t;

  }

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_data_handle_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
