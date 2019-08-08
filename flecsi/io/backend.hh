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
  implemenation for the io model.
 *----------------------------------------------------------------------------*/

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <flecsi/io/legion/io_policy.hh>

namespace flecsi {
namespace io {

using FLECSI_RUNTIME_IO_POLICY = legion_io_policy_t;

} // namespace io
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/io/mpi/io_policy.hh>

namespace flecsi {
namespace io {

using FLECSI_RUNTIME_IO_POLICY = mpi_io_policy_t;

} // namespace io
} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/io/hpx/io_policy.hh>

namespace flecsi {
namespace io {

using FLECSI_RUNTIME_IO_POLICY = hpx_io_policy_t;

} // namespace io
} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL
