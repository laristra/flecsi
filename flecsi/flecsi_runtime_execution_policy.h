/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_runtime_execution_policy_h
#define flecsi_runtime_execution_policy_h

///
/// \file
/// \date Initial file creation: Aug 01, 2016
///

///
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
///

#include "flecsi.h"

// Serial Policy
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

  #include "flecsi/execution/serial/execution_policy.h"

  namespace flecsi {
  namespace execution {

  using flecsi_execution_policy_t = serial_execution_policy_t;

  } // namespace execution
  } // namespace flecsi

// Legion Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

  #include "flecsi/execution/legion/execution_policy.h"

  namespace flecsi {
  namespace execution {

  using flecsi_execution_policy_t = legion_execution_policy_t;

  } // namespace execution
  } // namespace flecsi

// MPI Policy
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

  #include "flecsi/execution/mpi/execution_policy.h"

  namespace flecsi {
  namespace execution {

  using flecsi_execution_policy_t = mpi_execution_policy_t;

  } // namespace execution
  } // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL

#endif // flecsi_runtime_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
