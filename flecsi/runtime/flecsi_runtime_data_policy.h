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

/*----------------------------------------------------------------------------*
  This section works with the build system to select the correct runtime
  implemenation for the data model.
 *----------------------------------------------------------------------------*/

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/data/legion/policy.h>

namespace flecsi {
namespace data {

using FLECSI_RUNTIME_CLIENT_HANDLE_POLICY = legion_client_handle_policy_t;

} // namespace data
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/data/mpi/policy.h>

namespace flecsi {
namespace data {

using FLECSI_RUNTIME_CLIENT_HANDLE_POLICY = mpi_client_handle_policy_t;

} // namespace data
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/data/hpx/policy.h>

namespace flecsi {
namespace data {

using FLECSI_RUNTIME_CLIENT_HANDLE_POLICY = hpx_client_handle_policy_t;

} // namespace data
} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
