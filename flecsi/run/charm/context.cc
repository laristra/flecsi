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

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#define __FLECSI_PRIVATE__
#endif

#include "flecsi/exec/launch.hh"
#include "flecsi/exec/charm/task_wrapper.hh"
#include "flecsi/run/charm/context.hh"
#include "flecsi/run/charm/mapper.hh"
#include "flecsi/run/types.hh"
#include <flecsi/data.hh>

#include <mpi-interoperate.h>

#include "context.def.h"

namespace flecsi::run {

using namespace boost::program_options;
using exec::charm::task_id;

namespace charm {

ContextGroup::ContextGroup() {
  CkPrintf("Group created on %i\n", CkMyPe());
}

void ContextGroup::top_level_task() {
  std::cout << "Executing the top level task" << std::endl;
  context_t & context_ = context_t::instance();
  detail::data_guard(),
    context_.exit_status() = (*context_.top_level_action_)();
  if (CkMyPe() == 0) {
    CkStartQD(CkCallback(CkCallback::ckExit));
  }
}
}

//----------------------------------------------------------------------------//
// Implementation of context_t::initialize.
//----------------------------------------------------------------------------//

int
context_t::initialize(int argc, char ** argv, bool dependent) {

  if(dependent) {
    CharmBeginInit(argc, argv);
    if (CkMyPe() == 0) {
      cgProxy = charm::CProxy_ContextGroup::ckNew();
    }
    CharmFinishInit();
  } // if

  context::process_ = CkMyPe();
  context::processes_ = CkNumPes();

  auto status = context::initialize_generic(argc, argv, dependent);

  if(status != success && dependent) {
    CharmLibExit();
  } // if

  return status;
} // initialize

//----------------------------------------------------------------------------//
// Implementation of context_t::finalize.
//----------------------------------------------------------------------------//

void
context_t::finalize() {
  context::finalize_generic();

  if(context::initialize_dependent_) {
    CharmLibExit();
  } // if
} // finalize

//----------------------------------------------------------------------------//
// Implementation of context_t::start.
//----------------------------------------------------------------------------//

int
context_t::start(const std::function<int()> & action) {
  using namespace Legion;

  /*
    Store the top-level action for invocation from the top-level task.
   */

  top_level_action_ = &action;

  context::start();

  /*
    Legion command-line arguments.
   */

  // FIXME: This needs to be gotten from Charm
  context::threads_per_process_ = 1;
  context::threads_ = context::processes_ * context::threads_per_process_;

  if (context::process_ == 0) {
    cgProxy.top_level_task();
  }
  StartCharmScheduler();

  return context::exit_status();
} // context_t::start

//----------------------------------------------------------------------------//
// Implementation of context_t::handoff_to_mpi.
//----------------------------------------------------------------------------//

void
context_t::handoff_to_mpi(Legion::Context & ctx, Legion::Runtime * runtime) {
  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher handoff_to_mpi_launcher(
    task_id<exec::charm::verb<mpi_handoff>>,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  handoff_to_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, handoff_to_mpi_launcher);

  fm.wait_all_results(true);
} // context_t::handoff_to_mpi

//----------------------------------------------------------------------------//
// Implementation of context_t::wait_on_mpi.
//----------------------------------------------------------------------------//

Legion::FutureMap
context_t::wait_on_mpi(Legion::Context & ctx, Legion::Runtime * runtime) {
  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher wait_on_mpi_launcher(task_id<exec::charm::verb<mpi_wait>>,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  wait_on_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, wait_on_mpi_launcher);

  fm.wait_all_results(true);

  return fm;
} // context_t::wait_on_mpi

//----------------------------------------------------------------------------//
// Implementation of context_t::connect_with_mpi.
//----------------------------------------------------------------------------//

void
context_t::connect_with_mpi(Legion::Context &, Legion::Runtime *) {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  LegionRuntime::Arrays::Rect<1> launch_bounds(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(size - 1));

  context_t::instance().set_all_processes(launch_bounds);
} // context_t::connect_with_mpi

} // namespace flecsi::run
