/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
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

#include <flecsi/data/legion/data_policy.hh>

namespace flecsi {
namespace data {

using data_policy_t = legion_data_policy_t;

} // namespace data
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/data/mpi/data_policy.hh>

namespace flecsi {
namespace data {

using data_policy_t = mpi_data_policy_t;

} // namespace data
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/data/hpx/data_policy.hh>

namespace flecsi {
namespace data {

using data_policy_t = hpx_data_policy_t;

} // namespace data
} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
