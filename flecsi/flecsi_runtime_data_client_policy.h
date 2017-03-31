/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_data_client_policy_h
#define flecsi_runtime_data_client_policy_h

// FIXME: Remove rf_mpilegion after refactor
#include "flecsi.h"

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_rf_mpilegion

  #include "flecsi/data/legion/data_policy.h"
  #include "flecsi/data/legion/handle_policy.h"

  #define flecsi_data_policy_t legion_data_policy_t
  #define flecsi_handle_policy_t data::legion_handle_policy_t

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

  #include "flecsi/data/serial/data_policy.h"
  #include "flecsi/data/serial/handle_policy.h"

  #define flecsi_data_policy_t serial_data_policy_t
  #define flecsi_handle_policy_t data::serial_handle_policy_t

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
  #error "This policy is not yet implemented!"
#endif // FLECSI_RUNTIME_MODEL


#endif // flecsi_runtime_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
