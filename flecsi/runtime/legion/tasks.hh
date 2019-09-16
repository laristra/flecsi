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
#else
//#include "flecsi/execution/launch.hh"
#include "../backend.hh"
#endif

namespace flecsi::runtime {

/*----------------------------------------------------------------------------*
  Legion top-level task.
 *----------------------------------------------------------------------------*/

inline void
top_level_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {

  context_t & context_ = context_t::instance();

  /*
    Initialize MPI interoperability.
   */

  context_.connect_with_mpi(ctx, runtime);
  context_.wait_on_mpi(ctx, runtime);

#if 0
  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the topology registry.
  //
  // NOTE: This needs to be called before the field registry below because
  //       The topology callbacks register field callbacks with the field
  //       registry.
  //--------------------------------------------------------------------------//

  auto & topology_registry = context_.topology_registry();

  for(auto & c : topology_registry) {
    for(auto & d : c.second) {
      d.second.second(d.second.first);
    } // for
  } // for
#endif

#if 0 // FIXME: We don't need this anymore after the refactor
  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the field registry.
  //--------------------------------------------------------------------------//

  auto & field_registry = context_.field_registry();

  for(auto & c : field_registry) {
    for(auto & f : c.second) {
      f.second.second(f.first, f.second.first);
    } // for
  } // for
#endif

  context_.initialize_global_topology();
  context_.initialize_default_index_coloring();
  context_.initialize_default_index_topology();

  auto args = runtime->get_input_args();

  /*
    Invoke the FleCSI runtime top-level action.
   */

  context_.exit_status() = context_.top_level_action()(args.argc, args.argv);

  context_.finalize_global_topology();
  context_.finalize_default_index_topology();

  /*
    Finish up Legion runtime and fall back out to MPI.
   */

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // top_level_task

/*----------------------------------------------------------------------------*
  Other legion tasks.
 *----------------------------------------------------------------------------*/

/*!
 Interprocess communication to pass control to MPI runtime.

 @ingroup legion-runtime
 */

inline void
handoff_to_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

/*!
 Interprocess communication to wait for control to pass back to the Legion
 runtime.

 @ingroup legion-runtime
 */

inline void
wait_on_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  context_t::instance().wait_on_mpi();
} // handoff_to_mpi_task

/*!
  Interprocess communication to unset mpi execute state.

  @ingroup legion-runtime
*/

inline void
unset_call_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

#if defined(FLECSI_ENABLE_FLOG)

/*!
 @ingroup legion-runtime
*/

#include <flecsi/utils/flog/state.hh>

inline size_t
flog_reduction_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  return utils::flog::flog_t::instance().packets().size();
} // flog_reduction_task

inline void
flog_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {

  std::function<void()> bound_mpi_task = utils::flog::send_to_one;

  context_t::instance().set_mpi_task(bound_mpi_task);
  context_t::instance().set_mpi_state(true);
} // flog_mpi_task

#endif // FLECSI_ENABLE_FLOG

} // namespace flecsi::runtime
