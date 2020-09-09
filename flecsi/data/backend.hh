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

#include "flecsi/run/types.hh" // field_id_t

#include <cstddef>
#include <utility>

namespace flecsi::data {
// Use {} if unknown:
enum completeness { incomplete = -1, complete = 1 };
using size2 = std::pair<std::size_t, std::size_t>;
constexpr inline std::size_t logical_size = 1ul << 32;
} // namespace flecsi::data

/*----------------------------------------------------------------------------*
  This section works with the build system to select the correct runtime
  implemenation for the data model.
 *----------------------------------------------------------------------------*/

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include "flecsi/data/leg/policy.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <flecsi/data/mpi/policy.hh>

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include <flecsi/data/hpx/policy.hh>

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_charm

#include "flecsi/data/charm/policy.hh"

#endif // FLECSI_RUNTIME_MODEL
