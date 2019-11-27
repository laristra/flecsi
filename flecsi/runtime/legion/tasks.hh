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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

//#include "flecsi/execution/launch.hh"
#include "../backend.hh"

namespace flecsi::runtime {

/*----------------------------------------------------------------------------*
  Other legion tasks.
 *----------------------------------------------------------------------------*/

/*!
 Interprocess communication to pass control to MPI runtime.

 @ingroup legion-runtime
 */

inline void
handoff_to_mpi_task(const Legion::Task *,
  const std::vector<Legion::PhysicalRegion> &,
  Legion::Context,
  Legion::Runtime *) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

/*!
 Interprocess communication to wait for control to pass back to the Legion
 runtime.

 @ingroup legion-runtime
 */

inline void
wait_on_mpi_task(const Legion::Task *,
  const std::vector<Legion::PhysicalRegion> &,
  Legion::Context,
  Legion::Runtime *) {
  context_t::instance().wait_on_mpi();
} // handoff_to_mpi_task

#if defined(FLECSI_ENABLE_FLOG)

/*!
 @ingroup legion-runtime
*/

#include <flecsi/utils/flog/state.hh>

inline size_t
flog_reduction_task(const Legion::Task *,
  const std::vector<Legion::PhysicalRegion> &,
  Legion::Context,
  Legion::Runtime *) {
  return utils::flog::flog_t::instance().packets().size();
} // flog_reduction_task

inline void
flog_mpi_task(const Legion::Task *,
  const std::vector<Legion::PhysicalRegion> &,
  Legion::Context,
  Legion::Runtime *) {

  std::function<void()> bound_mpi_task = utils::flog::send_to_one;

  context_t::instance().set_mpi_task(bound_mpi_task);
} // flog_mpi_task

#endif // FLECSI_ENABLE_FLOG

} // namespace flecsi::runtime
