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

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {
namespace execution {

/*!
 This is the top-level initialization function to be defined by the
 FleCSI specialization layer. This symbol will be undefined in the compiled
 library, and is intended as a place holder for the specializations's
 initialization function that will resolve the missing symbol.

 The top-level initialization function is the first of the two control
 points that are exposed to the specialization. This function is
 responsible for adding specialization-specific inforamtion to the FleCSI
 runtime, e.g., named index spaces, adjacencies, etc. that must occur
 during the top-level task initialization stage.

 @param argc The number of arguments in argv (passed from the command line).
 @param argv The list of arguments (passed from the command line).

 @ingroup legion-execution
 */

#if defined FLECSI_ENABLE_SPECIALIZATION_TLT_INIT
void specialization_tlt_init(int argc, char ** argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT

/*! @cond IGNORE */

void driver(int argc, char ** argv);

/*! @endcond */

/*!
 The FleCSI runtime driver task. This is the top-level Legion task.

 @ingroup legion-execution
 */

void runtime_driver(
    const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context ctx,
    Legion::Runtime * runtime);

// The runtime_driver task is registered directly during context
// initialization.

/*!
 Initial SPMD task.

 @ingroup legion-execution
 */

void spmd_task(
    const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context ctx,
    Legion::Runtime * runtime);

} // namespace execution
} // namespace flecsi
