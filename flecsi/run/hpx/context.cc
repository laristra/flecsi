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

#include "flecsi/exec/hpx/task_wrapper.hh"
#include "flecsi/exec/launch.hh"
#include "flecsi/run/hpx/context.hh"
#include "flecsi/run/types.hh"
#include <flecsi/data.hh>

#include <hpx/hpx_init.hpp>
#include <hpx/runtime_distributed.hpp>

#include <memory>
#include <string>
#include <vector>

namespace flecsi::run {

using exec::hpx::task_id;

/*----------------------------------------------------------------------------*
  Startup task for HPX, runs top-level task and finalizes the runtime
  execution.
*----------------------------------------------------------------------------*/

int
top_level_task(int argc, char * argv[]) {

  context_t & context_ = context_t::instance();

  /*
    Initialize number of threads etc. from HPX
  */
  context::threads_per_process_ = ::hpx::get_os_thread_count();
  context::threads_ = context::processes_ * context::threads_per_process_;

  {
    /*
      Create and associate thread-specific data with top level HPX task
    */
    flecsi::run::hpx::runtime_data_wrapper wrap(FLECSI_TOP_LEVEL_TASK_ID);

    /*
      Initialize MPI interoperability.
     */

    //   context_.connect_with_mpi(ctx, runtime);
    //   context_.wait_on_mpi(ctx, runtime);

    /*
      Invoke the FleCSI runtime top-level action.
     */

    int result = (*context_.top_level_action_)();

    detail::data_guard(), context_.exit_status() = result;

    /*
      Reset MPI interoperability.
     */
    //   context_.handoff_to_mpi(ctx, runtime);
  }

  /*
    Finish up HPX runtime.
   */

  ::hpx::finalize();

  return result;
} // top_level_task

//----------------------------------------------------------------------------//
// Implementation of context_t::initialize.
//----------------------------------------------------------------------------//

int
context_t::initialize(int argc, char ** argv, bool dependent) {

  /*
    Store the command line arguments for invocation from the top-level task.
   */
  argc_ = argc;
  argv_ = argv_;

  if(dependent) {
    MPI_Init(&argc, &argv);
  } // if

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  context::process_ = rank;
  context::processes_ = size;

  auto status = context::initialize_generic(argc, argv, dependent);

  if(status != success && dependent) {
    MPI_Finalize();
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
    MPI_Finalize();
  } // if
} // finalize

//----------------------------------------------------------------------------//
// Implementation of context_t::start.
//----------------------------------------------------------------------------//

int
context_t::start(const std::function<int()> & action) {

  context::top_level_action_ = &action;

  context::start();

  /*
    Start HPX runtime. This also launches top level action
   */

  {
    log::devel_guard("context");

    std::stringstream stream;

    stream << "Starting HPX runtime" << std::endl;
    stream << "\targc: " << argc_ << std::endl;
    stream << "\targv: ";

    for(int i = 0; i != argc_; ++i) {
      stream << argv_[i] << " ";
    } // for

    stream << std::endl;

    flog_devel(info) << stream.str();
  } // scope

  ::hpx::init_params params;
  params.cfg = std::vector<std::string>{
    "hpx.hpx.run_hpx_main!=1", // run top-level task on all localities
    "hpx.commandline.allow_unknown!=1" // ignore unknown command line options
  };

  return ::hpx::init(top_level_task, argc_, argv_, params);
} // context_t::start

} // namespace flecsi::run
