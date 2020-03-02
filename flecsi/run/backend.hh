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

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include "flecsi/run/leg/context.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include "mpi/context.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include "hpx/context.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_charm

#include "charm/context.hh"

#endif // FLECSI_RUNTIME_MODEL

namespace flecsi::run {
context_t &
context::instance() {
  static context_t context;
  return context;
} // instance
} // namespace flecsi::run
