/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_data_client_policy_h
#define flecsi_runtime_data_client_policy_h

///
/// \file
/// \date Initial file creation: Mar 01, 2016
///

///
/// This section works with the build system to select the correct runtime
/// implemenation for the task model. If you add to the possible runtimes,
/// remember to edit config/packages.cmake to include a definition using
/// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
///

#include "flecsi.h"

// Serial Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

  #include "flecsi/data/serial/data_policy.h"
  #include "flecsi/data/serial/handle_policy.h"

  #define flecsi_data_policy_t serial_data_policy_t
  #define flecsi_handle_policy_t data::serial_handle_policy_t

// Legion Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/data/legion/data_policy.h"
  #include "flecsi/data/legion/handle_policy.h"

  #define flecsi_data_policy_t legion_data_policy_t
  #define flecsi_handle_policy_t data::legion_handle_policy_t

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #error "This policy is not yet implemented!"

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
