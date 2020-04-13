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

#include "flecsi/data/field.hh"
#include "flecsi/topo/core.hh"

namespace flecsi::io {
using field_reference_t = data::field_reference_t<topo::index>;
}

/*----------------------------------------------------------------------------*
  This section works with the build system to select the correct runtime
  implemenation for the io model.
 *----------------------------------------------------------------------------*/

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include "flecsi/io/leg/policy.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/io/mpi/policy.hh>

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/io/hpx/policy.hh>

#endif // FLECSI_RUNTIME_MODEL
